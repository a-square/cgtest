#include "config.h"

#if defined(_WIN32) && defined(HAVE_WINDOWS_H)
#   define UNICODE 1
#   define WIN32_LEAN_AND_MEAN
#   include <windows.h>
#endif

#if defined(HAVE_OPENGL_GL_H)
#   include <OpenGL/gl.h>
#   include <OpenGL/glext.h>
#elif defined(HAVE_GL_GL_H)
#   include <GL/gl.h>
#   include <GL/glext.h>
#else
#   error OpenGL not found!
#endif