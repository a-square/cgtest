#pragma once

#include <stdbool.h>
#include "opengl.h" // TODO: create smaller include

// log_printf

void log_printf(const char *format, ...)
    __attribute__((format (printf, 1, 2)));

// verify - abort unless an expression is boolean true

extern void abort();

#define verify(expr, format, ...) do { \
    if (!(expr)) { \
        log_printf(format, ##__VA_ARGS__); \
        abort(); \
    } \
} while(0)

// misc. verify specializations

void verify_posix(int errnum, const char *message);

void verify_gl();
void verify_gl_shader(GLuint shader);
void verify_gl_program(GLuint program);

// assert

#if defined(NDEBUG)
#   define assert(...) (void)0
#else
#   define assert(...) verify(__VA_ARGS__)
#endif

// make_window

typedef struct make_window_params {
    const char *title;
    int width, height;
    bool fullscreen;
    bool vsync;
} make_window_params_t;

typedef struct make_window_return {
    int width, height; // differs from params in high DPI modes
} make_window_return_t;

make_window_return_t make_window_p(make_window_params_t);
#define make_window(...) make_window_p((make_window_params_t){ \
    .title = "A window", \
    .width = 640, \
    .height = 480, \
    .fullscreen = false, \
    .vsync = false, \
    __VA_ARGS__ \
})

// destroy_window

void destroy_window();

// swap_window_buffers

void swap_window_buffers();