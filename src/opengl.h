#include "config.h"

#if defined(HAVE_OPENGL_GL_H)
#   include <OpenGL/gl3.h>
#elif defined(HAVE_GL_GL_H)
#   include <GL/gl3.h>
#else // provide OpenGL yourself
#   include <gl3.h>
#endif