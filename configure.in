dnl Process this file with autoconf to produce a configure script.
AC_INIT(nct.c)
AM_INIT_AUTOMAKE(nct, 1.4)
AM_CONFIG_HEADER(config.h)

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

dnl Checks for libraries.
AC_CHECK_LIB(ncurses, initscr,
   [LIBS="$LIBS -lncurses"],
   [AC_CHECK_LIB(curses, main)])

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(fcntl.h sys/time.h unistd.h ncurses.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_TIME

dnl Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS(gettimeofday use_default_colors)

if test $prefix = /usr -a $localstatedir = '${prefix}/var'; then
   localstatedir = /var
fi

AC_OUTPUT(Makefile)
