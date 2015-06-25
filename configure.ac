#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(mod_fluentd, 0.2.0,[])

AC_CONFIG_SRCDIR([mod_fluentd.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([-Wall -Werror foreign subdir-objects])

# Checks for programs.
# AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL

# Option for fluentd debug log level
AC_ARG_WITH(fluentd-debug-log,
  AC_HELP_STRING([--with-fluentd-debug-log=LEVEL],
    [fluentd debug log level [default=no]]),
  [FLUENTD_DEBUG_LOG="${withval}"],
  [FLUENTD_DEBUG_LOG=no]
)
AS_IF([test "x${FLUENTD_DEBUG_LOG}" != xno],
    [AC_DEFINE_UNQUOTED([AP_FLUENTD_DEBUG_LOG_LEVEL],
      [${FLUENTD_DEBUG_LOG}], [fluentd debug log level])]
)

# Checks for apxs.
AC_ARG_WITH(apxs,
  [AC_HELP_STRING([--with-apxs=PATH], [apxs path [default=yes]])],
  [APXS_PATH="$withval"],
  [APXS_PATH="/usr/bin:/usr/sbin:/usr/local/bin:/usr/local/sbin"]
)

AC_PATH_PROG(APXS, apxs, no, ${APXS_PATH})

AS_IF([test "x${APXS}" = "xno"], AC_MSG_ERROR([apxs not found]),
  [test "x${APXS}" = "x"], AC_MSG_ERROR([apxs not found])
)

APXS_CFLAGS=`${APXS} -q CFLAGS 2> /dev/null`
APXS_CPPFLAGS=`${APXS} -q CPPFLAGS 2> /dev/null`
APXS_LDFLAGS=`${APXS} -q LDFLAGS 2> /dev/null`
APXS_LIBS=`${APXS} -q LIBS 2> /dev/null`
APXS_LIBEXECDIR=`${APXS} -q LIBEXECDIR 2> /dev/null`

APXS_INCLUDEDIR=`${APXS} -q INCLUDEDIR 2> /dev/null`
APXS_INCLUDES="-I${APXS_INCLUDEDIR}"

# Checks for apr.
AC_ARG_WITH(apr,
  [AC_HELP_STRING([--with-apr=PATH], [apr config path [default=yes]])],
  [APR_CONFIG="$withval"],
  [with_apr=yes]
)

AC_MSG_CHECKING([whether apr])
AS_IF([test "x$with_apr" != "xno"],
  [
   if test ! -x "${APR_CONFIG}" -o -z "${APR_CONFIG}"; then
     APR_PATH=`${APXS} -q APR_BINDIR 2> /dev/null`
     APR_CONFIG="${APR_PATH}/apr-1-config"
     if test ! -x "${APR_CONFIG}"; then
       APR_CONFIG="${APR_PATH}/apr-config"
     fi
   fi
   APR_INCLUDES=`${APR_CONFIG} --includes 2> /dev/null`
   APR_CFLAGS=`${APR_CONFIG} --cflags 2> /dev/null`
   APR_CPPFLAGS=`${APR_CONFIG} --cppflags 2> /dev/null`
   APR_LDFLAGS=`${APR_CONFIG} --ldflags 2> /dev/null`
   APR_LIBS=`${APR_CONFIG} --libs 2> /dev/null`
   AC_MSG_RESULT(yes)
  ],
  AC_MSG_ERROR(apr not found)
)

# Apache libraries.
APACHE_MODULEDIR="${APXS_LIBEXECDIR}"
APACHE_INCLUDES="${APXS_INCLUDES} ${APR_INCLUDES}"
APACHE_CFLAGS="${APXS_CFLAGS} ${APR_CFLAGS} ${APACHE_INCLUDES}"
APACHE_CPPFLAGS="${APXS_CPPFLAGS} ${APR_CPPFLAGS} ${APACHE_INCLUDES}"
APACHE_LDFLAGS="${APXS_LDFLAGS} ${APR_LDFLAGS}"
APACHE_LIBS="${APXS_LIBS} ${APR_LIBS}"

AC_SUBST(APACHE_MODULEDIR)
AC_SUBST(APACHE_INCLUDES)
AC_SUBST(APACHE_CFLAGS)
AC_SUBST(APACHE_CPPFLAGS)
AC_SUBST(APACHE_LDFLAGS)
AC_SUBST(APACHE_LIBS)

# Jansson libraries.
AC_CHECK_HEADERS([endian.h fcntl.h locale.h sched.h unistd.h sys/param.h sys/stat.h sys/time.h sys/types.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_UINT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT8_T
AC_TYPE_LONG_LONG_INT

AC_C_INLINE
case $ac_cv_c_inline in
    yes) json_inline=inline;;
    no) json_inline=;;
    *) json_inline=$ac_cv_c_inline;;
esac
AC_SUBST([json_inline])

# Checks for library functions.
AC_CHECK_FUNCS([close getpid gettimeofday localeconv open read sched_yield strtoll])

AC_MSG_CHECKING([for gcc __sync builtins])
have_sync_builtins=no
AC_TRY_LINK(
  [], [unsigned long val; __sync_bool_compare_and_swap(&val, 0, 1);],
  [have_sync_builtins=yes],
)
if test "x$have_sync_builtins" = "xyes"; then
  AC_DEFINE([HAVE_SYNC_BUILTINS], [1],
    [Define to 1 if gcc's __sync builtins are available])
fi
AC_MSG_RESULT([$have_sync_builtins])

AC_MSG_CHECKING([for gcc __atomic builtins])
have_atomic_builtins=no
AC_TRY_LINK(
  [], [char l; unsigned long v; __atomic_test_and_set(&l, __ATOMIC_RELAXED); __atomic_store_n(&v, 1, __ATOMIC_RELEASE); __atomic_load_n(&v, __ATOMIC_ACQUIRE);],
  [have_atomic_builtins=yes],
)
if test "x$have_atomic_builtins" = "xyes"; then
  AC_DEFINE([HAVE_ATOMIC_BUILTINS], [1],
    [Define to 1 if gcc's __atomic builtins are available])
fi
AC_MSG_RESULT([$have_atomic_builtins])

case "$ac_cv_type_long_long_int$ac_cv_func_strtoll" in
     yesyes) json_have_long_long=1;;
     *) json_have_long_long=0;;
esac
AC_SUBST([json_have_long_long])

case "$ac_cv_header_locale_h$ac_cv_func_localeconv" in
     yesyes) json_have_localeconv=1;;
     *) json_have_localeconv=0;;
esac
AC_SUBST([json_have_localeconv])

JANSSON_CFLAGS="-I./jansson/src"
JANSSON_CPPFLAGS="${JANSSON_CFLAGS}"
JANSSON_LIBS=
JANSSON_LDFLAGS=

AC_SUBST(JANSSON_CFLAGS)
AC_SUBST(JANSSON_CPPFLAGS)
AC_SUBST(JANSSON_LDFLAGS)
AC_SUBST(JANSSON_LIBS)

AC_CONFIG_FILES([
    Makefile
    jansson/src/Makefile
    jansson/src/jansson_config.h
])
AC_OUTPUT
