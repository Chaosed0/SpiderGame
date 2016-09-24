#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <functional>

#ifdef WIN32
#define GLDEBUGOUTPUT_PREFIX APIENTRY
#else
#define GLDEBUGOUTPUT_PREFIX
#endif

void GLDEBUGOUTPUT_PREFIX glDebugOutput(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);
GLenum glCheckError_(const char *file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__) 
