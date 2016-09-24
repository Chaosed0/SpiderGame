#pragma once

#include <SDL.h>

#include <memory>
#include <vector>
#include <string>

struct TextureImpl;
struct Texture
{
	Texture();
	Texture(const Texture& texture);
	std::unique_ptr<TextureImpl> impl;
};

typedef enum {
	TextureType_diffuse = 0,
	TextureType_specular,
	TextureType_cubemap
} TextureType;

struct TextureLoader
{
	Texture loadFromFile(TextureType type, const std::string& imageLocation);
	Texture loadCubemap(const std::vector<std::string>& images);
};