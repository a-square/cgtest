BINARIES=barebone tunnel

DISABLED_WARNINGS = -Wno-initializer-overrides
CFLAGS = -Wall -Wextra $(DISABLED_WARNINGS) -g -O3 -pthread -std=gnu11 `pkg-config --cflags sdl2`
LDLIBS = `pkg-config --libs sdl2`

# OS X OpenGL
CFLAGS += -I/System/Library/Frameworks/OpenGL.framework/Headers
LDLIBS += -L/System/Library/Frameworks/OpenGL.framework/Libraries -lGL

all: $(BINARIES)

barebone: barebone.o main-oldschool.o util.o

tunnel: tunnel.o main-oldschool.o util.o

clean:
	rm -f *.o $(BINARIES)

.PHONY: clean