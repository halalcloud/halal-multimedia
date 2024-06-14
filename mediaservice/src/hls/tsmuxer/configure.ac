#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libmpeg2],[1.0],[BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([bitstreamfilter.cpp])
AC_CONFIG_HEADERS([config.h])
AM_INIT_AUTOMAKE
LT_INIT

PKG_CHECK_MODULES([DEPS], [libavcodec libavformat libavutil libswscale libswresample x264])

AC_ARG_WITH([mysql-include-path], [AS_HELP_STRING([--with-mysql-include-path], [location of the mysql headers, defaults to /usr/include/mysql])]
	,[MYSQL_CFLAGS="-I$withval"],
	,[MYSQL_CFLAGS='-I/usr/include/mysql'])
AC_SUBST([MYSQL_CFLAGS])

AC_CONFIG_MACRO_DIR([m4])
# Checks for programs.
AC_PROG_CXX
#AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_REALLOC
AC_CHECK_FUNCS([memset strstr])

AC_CONFIG_FILES([Makefile mpeg2.pc])
AC_OUTPUT
