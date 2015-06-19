#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cgtest], [0.1.0], [lex.aver@gmail.com])
AM_INIT_AUTOMAKE
AM_CFLAGS=-Wall -Wextra -Wno-initializer-overrides -g -O3 -std=gnu11 -pthread
AC_CONFIG_SRCDIR([barebone.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC

# Check for modules
PKG_CHECK_MODULES([SDL2], [sdl2])

# Checks for libraries.
AX_CHECK_GL

# Checks for header files.
AC_CHECK_HEADERS([locale.h stddef.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_C_RESTRICT
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday setlocale strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

