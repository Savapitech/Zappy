#pragma once

#if defined(__APPLE__)
    #include <OpenGL/gl3.h>
    #include <OpenGL/gl3ext.h>
#else
    #define GL_GLEXT_PROTOTYPES 1
    #include <GL/gl.h>
#endif