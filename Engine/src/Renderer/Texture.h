#pragma once

#include <GL/glew.h>

#include "Renderer/TextureLoader.h"

struct TextureImpl
{
	TextureImpl();
	TextureImpl(TextureType type, GLuint id);
	TextureType type;
	GLuint id;
};