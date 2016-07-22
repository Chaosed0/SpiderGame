
#include "RenderUtil.h"

#include <cstdio>
#include <sstream>

void GLDEBUGOUTPUT_PREFIX glDebugOutput(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar *message,
	const void *userParam)
{
	// ignore non-significant error/warning codes
	//if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;
	std::stringstream sstream;

	sstream << "GL Debug: (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             sstream << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   sstream << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: sstream << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     sstream << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     sstream << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           sstream << "Source: Other"; break;
	} sstream << " | ";

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               sstream << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: sstream << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  sstream << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         sstream << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         sstream << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              sstream << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          sstream << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           sstream << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               sstream << "Type: Other"; break;
	} sstream << " | ";

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         sstream << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       sstream << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          sstream << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: sstream << "Severity: notification"; break;
	} sstream << std::endl;

	if (userParam != nullptr) {
		DebugLogCallback* callback = (DebugLogCallback*)userParam;
		(*callback)(sstream.str());
	}
	fprintf(stderr, sstream.str().c_str());
	fprintf(stderr, "\n");
}

GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR)
	{
		char* error = "";
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		fprintf(stderr, "%s|%s:%d\n", error, file, line);
	}
	return errorCode;
}
