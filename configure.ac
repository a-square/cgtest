#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([cgtest], [0.1.0], [lex.aver@gmail.com])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])
AC_CONFIG_SRCDIR([src/util.c])
AC_CONFIG_HEADERS([config.h])

# Checks for compilers
AC_PROG_CC([clang gcc c99])
AC_PROG_CC_STDC

# Checks for SDL
SDL2_VERSION=2.0.0
AM_PATH_SDL2($SDL2_VERSION,
	:,
	AC_MSG_ERROR([*** SDL version $SDL_VERSION not found!])
)

# Checks for OpenGL
AX_CHECK_GL

# Checks for GLEW
GLEW_CFLAGS=""
GLEW_LIBS="-lGLEW"
PKG_CHECK_MODULES(GLEW, [glew], [], [
		  AC_CHECK_HEADER([GL/glew.h],
		  [],
		  AC_MSG_ERROR([GLEW required]))
		  AC_CHECK_LIB([GLEW],
			       [glewInit],
			       [],
			       AC_MSG_ERROR([GLEW required]))
		  ])


# Checks for libm
AC_CHECK_LIB([m], [cos])

# Checks for header files.
AC_CHECK_HEADERS([locale.h stddef.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
#AC_CHECK_HEADER_STDBOOL
#AC_C_INLINE
#AC_C_RESTRICT
#AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday setlocale strerror])

AC_CONFIG_FILES([Makefile])
AC_OUTPUT

