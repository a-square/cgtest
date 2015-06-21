ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = $(SDL2_CFLAGS) $(GL_CFLAGS) -Wall -Wextra -Wno-initializer-overrides
AM_LDFLAGS = $(SDL2_LIBS) $(GL_LIBS)

bin_PROGRAMS = barebone tunnel

barebone_SOURCES = src/barebone.c src/main-oldschool.c src/util.c
tunnel_SOURCES = src/tunnel.c src/main-oldschool.c src/util.c