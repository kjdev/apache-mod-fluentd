/*
 * mod_fluentd.c -- Apache fluentd module
 *
 * Then activate it in Apaches' httpd.conf file:
 *
 *   # httpd.conf
 *   LoadModule fluentd_module modules/mod_fluentd.so
 *
 *   CustomLog fluentd:tag combined
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#  undef PACKAGE_NAME
#  undef PACKAGE_STRING
#  undef PACKAGE_TARNAME
#  undef PACKAGE_VERSION
#endif

#ifdef __GNUC__
#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#else
#  define UNUSED(x) UNUSED_ ## x
#endif

/* httpd */
#include "ap_config.h"
#include "apr_strings.h"
#include "mod_log_config.h"
#include "httpd.h"
#include "http_config.h"
/* socket */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
/* json */
#include "jansson.h"

/* log */
#include "http_log.h"
#ifdef AP_FLUENTD_DEBUG_LOG_LEVEL
#define FLUENTD_DEBUG_LOG_LEVEL AP_FLUENTD_DEBUG_LOG_LEVEL
#else
#define FLUENTD_DEBUG_LOG_LEVEL APLOG_DEBUG
#endif
#define _DEBUG(p, format, args...)                                      \
    ap_log_perror(APLOG_MARK, FLUENTD_DEBUG_LOG_LEVEL, 0,               \
                  p, "[FLUENTD] %s(%d): "format, __FILE__, __LINE__, ##args)

/* default */
#define DEFAULT_FLUENTD_HOST "127.0.0.1"
#define DEFAULT_FLUENTD_PORT 5160

#define PREFIX_FLUENTD "fluentd:"
#define PREFIX_FLUENTD_LENGTH 8

module AP_MODULE_DECLARE_DATA fluentd_module;

static APR_OPTIONAL_FN_TYPE(ap_log_set_writer_init) *log_writer_init;
static APR_OPTIONAL_FN_TYPE(ap_log_set_writer) *log_writer;

static ap_log_writer_init *default_log_writer_init = NULL;
static ap_log_writer *default_log_writer = NULL;

typedef struct {
    char *dummy;
    int sockfd;
    struct sockaddr_in addr;
    char *tag;
    char *host;
    char *extend;
    int port;
} fluentd_handle_t;

char fluentd_dummy_handle[16];

static void *
fluentd_writer_init(apr_pool_t *p, server_rec *s, const char * name)
{
    _DEBUG(p, "name = %s", name);

    if (strncasecmp(PREFIX_FLUENTD, name, PREFIX_FLUENTD_LENGTH) == 0) {
        char *format = apr_pstrdup(p, name);
        char *host = DEFAULT_FLUENTD_HOST;
        char *c = NULL, *tag = NULL, *extend = NULL;
        int i = 0, port = DEFAULT_FLUENTD_PORT;
        fluentd_handle_t *handle;

        tag = format + PREFIX_FLUENTD_LENGTH;

        if ((c = strchr(format, '#')) != NULL) {
            i = c - format;
            format[i++] = '\0';
            host = format + i;
            if ((c = strchr(format + i, '@')) != NULL) {
                i = c - format;
                format[i++] = '\0';
                port = atoi(format + i);
            }
        }

        if ((c = strchr(format + i, ' ')) != NULL) {
            i = c - format;
            format[i++] = '\0';
            extend = format + i;
        }

        _DEBUG(p, "fluentd: tag = %s, host = %s, port = %d, extend = %s",
               tag, host, port, extend);

        handle = (fluentd_handle_t *)apr_palloc(p, sizeof(fluentd_handle_t));

        handle->dummy = &fluentd_dummy_handle[0];
        handle->host = host;
        handle->tag = tag;
        handle->port = port;
        handle->extend = extend;
        handle->addr.sin_family = AF_INET;
        handle->addr.sin_port = htons(port);
        handle->addr.sin_addr.s_addr = inet_addr(host);
        handle->sockfd = socket(AF_INET, SOCK_DGRAM, 0);

        return handle;
    }

    if (default_log_writer_init) {
        return default_log_writer_init(p, s, name);
    }

    return NULL;
}

static apr_status_t
fluentd_writer(request_rec *r, void *handle, const char **strs, int *strl,
               int nelts, apr_size_t len)
{
    fluentd_handle_t *fluentd = (fluentd_handle_t *)handle;

    if (fluentd->dummy == fluentd_dummy_handle) {
        char *str;
        char *s;
        int i;
        char *message = NULL;
        json_t *msg;
        /* apr_status_t rv; */

        str = apr_palloc(r->pool, len + 1);

        for (i = 0, s = str; i < nelts; ++i) {
            memcpy(s, strs[i], strl[i]);
            s += strl[i];
        }
        str[len] = '\0';

        _DEBUG(r->pool, "fluentd: tag = %s, host = %s, port = %d",
               fluentd->tag, fluentd->host, fluentd->port);

        msg = json_object();
        json_object_set_new(msg, "message", json_string(str));
        json_object_set_new(msg, "tag", json_string(fluentd->tag));

        if (fluentd->extend) {
            json_t *data = NULL;
            json_error_t error;

            data = json_loads(fluentd->extend, 0, &error);
            if (data && json_is_object(data)) {
                const char *key;
                json_t *value;
                void *iter = json_object_iter(data);
                while (iter) {
                    key = json_object_iter_key(iter);
                    value = json_object_iter_value(iter);

                    json_object_set_new(msg, key, value);

                    iter = json_object_iter_next(data, iter);
                }
            } else {
                json_delete(data);
            }
        }

        message = json_dumps(msg,
                             JSON_COMPACT|JSON_ENCODE_ANY|JSON_ENSURE_ASCII);
        json_delete(msg);

        if (message) {
            sendto(fluentd->sockfd, message, strlen(message), 0,
                   (struct sockaddr *)&(fluentd->addr), sizeof(fluentd->addr));
            free(message);
        }

        return OK;
    }

    if (default_log_writer) {
        return default_log_writer(r, handle, strs, strl, nelts, len);
    }

    return OK;
}

static int
fluentd_pre_config(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp)
{
    if (!log_writer_init) {
        log_writer_init = APR_RETRIEVE_OPTIONAL_FN(ap_log_set_writer_init);
        log_writer = APR_RETRIEVE_OPTIONAL_FN(ap_log_set_writer);
    }

    if (!default_log_writer_init) {
        void *logger;

        logger = log_writer_init(fluentd_writer_init);
        if (logger != fluentd_writer_init) {
            default_log_writer_init = logger;
        }

        logger = log_writer(fluentd_writer);
        if (logger != fluentd_writer) {
            default_log_writer = logger;
        }
    }

    return OK;
}

static void
fluentd_register_hooks(apr_pool_t * UNUSED(p))
{
    static const char *pre[] = { "mod_log_config.c", NULL };
    ap_hook_pre_config(fluentd_pre_config, pre, NULL, APR_HOOK_REALLY_LAST);
}


module AP_MODULE_DECLARE_DATA fluentd_module =
{
    STANDARD20_MODULE_STUFF,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    fluentd_register_hooks
};
