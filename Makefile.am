ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = $(SDL_CFLAGS) $(GLEW_CFLAGS) $(GL_CFLAGS) -Wall -Wextra -Wno-initializer-overrides

bin_PROGRAMS = barebone tunnel

barebone_SOURCES = src/barebone.c src/main-oldschool.c src/util.c
tunnel_SOURCES = src/tunnel.c src/main-oldschool.c src/util.c

LDADD = $(SDL_LIBS) $(GLEW_LIBS) $(GL_LIBS) -lm

EXTRA_DIST = src/opengl.h src/oldschool.h src/util.h