dnl Process this file with autoconf to produce a configure script.
AC_INIT([nct], [1.4a], [lavv17f@gmail.com])
AC_CONFIG_SRCDIR([src/nct.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
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

if test x$GCC = xyes; then
   CFLAGS="$CFLAGS -Wall"
fi

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/time.h unistd.h)

dnl Checks for libraries.
AX_WITH_CURSES
if test "x$ax_cv_curses" != xyes; then
	AC_MSG_ERROR([cannot find (n)curses library])
fi
LIBS="$LIBS $CURSES_LIB"

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

dnl Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS([gettimeofday use_default_colors])

if test $prefix = /usr -a $localstatedir = '${prefix}/var'; then
   localstatedir = /var
fi

AC_CONFIG_FILES([Makefile m4/Makefile src/Makefile])
AC_OUTPUT
