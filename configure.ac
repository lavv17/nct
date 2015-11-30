dnl Process this file with autoconf to produce a configure script.
AC_INIT([nct], [1.4a], [lavv17f@gmail.com])
AC_CONFIG_SRCDIR([src/nct.c])
AM_CONFIG_HEADER([lib/config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_LIBOBJ_DIR([lib])
AM_INIT_AUTOMAKE

WITH_DEBUG=no
AC_ARG_WITH(debug,
   [  --with-debug            enable debug info],
   [WITH_DEBUG="$withval"])

if test x$WITH_DEBUG = xno; then
   : ${CFLAGS=-O} ${LDFLAGS=-s}
fi

dnl Checks for programs.
AC_PROG_CC
gl_EARLY

if test x$GCC = xyes; then
   CFLAGS="$CFLAGS -Wall"
fi

gl_INIT

dnl Checks for libraries.
AX_WITH_CURSES

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

dnl Checks for library functions.
AC_FUNC_MMAP

LIBS="$LIBS $CURSES_LIB"
AC_CHECK_FUNCS([use_default_colors])

if test $prefix = /usr -a $localstatedir = '${prefix}/var'; then
   localstatedir = /var
fi

AC_CONFIG_FILES([Makefile m4/Makefile lib/Makefile src/Makefile])
AC_OUTPUT
