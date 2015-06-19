#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <sys/time.h>
#include <SDL.h>
#include <gl3.h>

// just one window is needed
static SDL_Window *g_window = NULL;
static SDL_GLContext g_context = NULL;

void verify_gl() {    
    GLenum error = glGetError();
    if (error) {
        const char *errorString = NULL;
        
        switch (error) {
        case GL_INVALID_ENUM:
            errorString = "Invalid enum";
            break;
        case GL_INVALID_VALUE:
            errorString = "Invalid value";
            break;
        case GL_INVALID_OPERATION:
            errorString = "Invalid operation";
            break;
        case GL_OUT_OF_MEMORY:
            errorString = "Out of memory";
            break;
        default:
            errorString = "UNKNOWN ERROR";
        }
        
        log_printf("OpenGL error: %s", errorString);
        abort();
    }
}

void verify_gl_shader(GLuint shader) {
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status); verify_gl();
    if (status == GL_FALSE) {
        GLint logLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength); verify_gl();
        char logContents[logLength];
        glGetShaderInfoLog(shader, logLength, &logLength, logContents); verify_gl();
        log_printf("OpenGL shader compilation error:\n%s", logContents);
        abort();
    }
}

void verify_gl_program(GLuint program) {
    GLint status;
    glGetProgramiv(program, GL_LINK_STATUS, &status); verify_gl();
    if (status == GL_FALSE) {
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength); verify_gl();
        char logContents[logLength];
        glGetProgramInfoLog(program, logLength, &logLength, logContents); verify_gl();
        log_printf("OpenGL program linking error:\n%s", logContents);
        abort();
    }
}

void log_printf(const char *format, ...) {
    // form the timestamp
    char time_str[64];
    struct timeval tv;
    gettimeofday(&tv, NULL);
    struct tm *tm = localtime(&tv.tv_sec);
    size_t offset = strftime(time_str, sizeof(time_str), "%F %T", tm);
    sprintf(time_str + offset, ".%03d", tv.tv_usec / 1000);
    
    // form the message
    va_list args;
    va_start(args, format);
    char *message;
    vasprintf(&message, format, args);
    va_end(args);
    
    // print and then deallocate the message
    fprintf(stderr, "[%s] %s\n", time_str, message);
    free(message);
}

make_window_return_t make_window_p(make_window_params_t p) {
    assert(!g_window, "Tried to create a window twice");
    SDL_Init(SDL_INIT_VIDEO);
    
    SDL_WindowFlags flags = 0
        | SDL_WINDOW_OPENGL
        | SDL_WINDOW_ALLOW_HIGHDPI
    ;
    
    if (p.fullscreen) {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        SDL_DisplayMode dm;
        verify(
            SDL_GetDesktopDisplayMode(0, &dm) == 0,
            "Could not query desktop resolution: %s", SDL_GetError()
        );
        p.width = dm.w;
        p.height = dm.h;
    }
    
    g_window = SDL_CreateWindow(
        p.title,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        p.width,
        p.height,
        flags
    );
    verify(g_window, "Could not create a %d⨉%d window: %s", p.width, p.height, SDL_GetError());
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    g_context = SDL_GL_CreateContext(g_window);
    verify(g_context, "Could not initialize OpenGL: %s", SDL_GetError());
    
    if (p.vsync) {
        verify(
            SDL_GL_SetSwapInterval(-1) == 0 || SDL_GL_SetSwapInterval(1) == 0,
            "Could not set the swap interval: %s", SDL_GetError()
        );
        SDL_ClearError();
    } else {
        verify(
            SDL_GL_SetSwapInterval(0) == 0,
            "Could not set the swap interval: %s", SDL_GetError()
        );
    }
    
    make_window_return_t r;
    SDL_GL_GetDrawableSize(g_window, &r.width, &r.height);
    
    if (p.fullscreen) {
        log_printf("Created a fullscreen window; drawable size: %d⨉%d", r.width, r.height);
    } else {
        log_printf(
            "Created a %d⨉%d window; drawable size: %d⨉%d",
            p.width, p.height, r.width, r.height
        );
    }
    log_printf("OpenGL v%s", glGetString(GL_VERSION));
    return r;
}

void destroy_window() {
    SDL_GL_DeleteContext(g_context);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
}

void swap_window_buffers() {
    SDL_GL_SwapWindow(g_window);
}