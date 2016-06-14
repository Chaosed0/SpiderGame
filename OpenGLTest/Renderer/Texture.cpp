
#include "Texture.h"
#include "RenderUtil.h"

#include <SDL.h>
#include <SDL_Image.h>

#include <cstdio>

Texture::Texture()
{}

Texture::Texture(TextureType type, const std::string& imageLocation)
{
	this->type = type;
	this->loadFromFile(imageLocation);
}

unsigned int imageColorMode(SDL_PixelFormat* format)
{
	unsigned int mode = 0;
	if (format->BytesPerPixel == 3) {
		mode = GL_RGB;
	} else if (format->BytesPerPixel == 4) {
		mode = GL_RGBA;
	} else if (format->BytesPerPixel == 1 && format->BitsPerPixel == 8) {
		mode = GL_COLOR_INDEX;
	}

	return mode;
}

bool Texture::loadFromFile(const std::string& imageLocation)
{
	SDL_Surface* texture = IMG_Load(imageLocation.c_str());
	if (texture == NULL) {
		fprintf(stderr, "Could not load texture %s: %s\n", imageLocation.c_str(), IMG_GetError());
		return false;
	}

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glCheckError();

	// Set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// Set texture wrapping to GL_REPEAT (usually basic wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// Set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glCheckError();

	unsigned int mode = imageColorMode(texture->format);
	glTexImage2D(GL_TEXTURE_2D, 0, mode, texture->w, texture->h, 0, mode, GL_UNSIGNED_BYTE, texture->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glCheckError();

	glBindTexture(GL_TEXTURE_2D, 0);
	
	return true;
}

bool Texture::loadCubemap(const std::vector<std::string>& images)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	glCheckError();

	for (unsigned int i = 0; i < images.size(); i++) {
		SDL_Surface* texture = IMG_Load(images[i].c_str());
		if (texture == NULL) {
			fprintf(stderr, "Could not load texture %s\n", images[i].c_str());
			return false;
		}

		unsigned int mode = imageColorMode(texture->format);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			mode, texture->w, texture->h, 0, mode, GL_UNSIGNED_BYTE, texture->pixels
			);
		glCheckError();
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glCheckError();

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	return true;
}
