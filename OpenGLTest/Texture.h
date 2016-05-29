#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <SDL.h>

#include <string>
#include <vector>

typedef enum {
	TextureType_diffuse = 0,
	TextureType_specular,
	TextureType_cubemap
} TextureType;

struct Texture
{
	Texture();
	Texture(TextureType type, const std::string& imageLocation);
	bool loadFromFile(const std::string& imageLocation);
	bool loadCubemap(const std::vector<std::string>& images);
	TextureType type;
	GLuint id;
};