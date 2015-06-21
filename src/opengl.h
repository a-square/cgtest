#include "config.h"

#if defined(_WIN32) && defined(HAVE_WINDOWS_H)
#   define UNICODE 1
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

#if defined(__APPLE__)
#   include <OpenGL/gl3.h>
#elif defined(HAVE_OPENGL_GLCOREARB_H)
#   include <OpenGL/glcorearb.h>
#elif defined(HAVE_GL_GLCOREARB_H)
#   include <GL/glcorearb.h>
#elif defined(HAVE_GLCOREARB_H)
#   include <glcorearb.h>
#else
#   error OpenGL not found!
#endif