
ACLOCAL_AMFLAGS = -I m4

moddir = @APACHE_MODULEDIR@
mod_LTLIBRARIES = mod_fluentd.la

mod_fluentd_la_SOURCES = \
	mod_fluentd.c \
	jansson/src/dump.c \
	jansson/src/error.c \
	jansson/src/hashtable.c \
	jansson/src/hashtable.h \
	jansson/src/hashtable_seed.c \
	jansson/src/jansson_private.h \
	jansson/src/load.c \
	jansson/src/lookup3.h \
	jansson/src/memory.c \
	jansson/src/pack_unpack.c \
	jansson/src/strbuffer.c \
	jansson/src/strbuffer.h \
	jansson/src/strconv.c \
	jansson/src/utf.c \
	jansson/src/utf.h \
	jansson/src/value.c

mod_fluentd_la_CFLAGS = @APACHE_CFLAGS@ @JANSSON_CFLAGS@
mod_fluentd_la_CPPFLAGS = @APACHE_CPPFLAGS@ @JANSSON_CPPFLAGS@
mod_fluentd_la_LDFLAGS = -avoid-version -module @APACHE_LDFLAGS@ @JANSSON_LDFLAGS@
mod_fluentd_la_LIBS = @APACHE_LIBS@ @JANSSON_LIBS@
