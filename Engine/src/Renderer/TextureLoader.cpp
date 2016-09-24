
#include "Renderer/TextureLoader.h"

#include "Renderer/Texture.h"
#include "Renderer/RenderUtil.h"

#include <SDL.h>
#include <SDL_Image.h>

#include <cstdio>

Texture::Texture() : impl(new TextureImpl()) { }
Texture::Texture(const Texture& texture) : impl(new TextureImpl(*texture.impl)) { }

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

Texture TextureLoader::loadFromFile(TextureType type, const std::string& imageLocation)
{
	Texture texture;
	texture.impl = std::make_unique<TextureImpl>();
	texture.impl->type = type;

	SDL_Surface* sdlTexture = IMG_Load(imageLocation.c_str());
	if (sdlTexture == NULL) {
		fprintf(stderr, "Could not load texture %s: %s\n", imageLocation.c_str(), IMG_GetError());
		texture.impl->id = 0;
		return texture;
	}

	GLuint id;
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

	unsigned int mode = imageColorMode(sdlTexture->format);
	glTexImage2D(GL_TEXTURE_2D, 0, mode, sdlTexture->w, sdlTexture->h, 0, mode, GL_UNSIGNED_BYTE, sdlTexture->pixels);
	glGenerateMipmap(GL_TEXTURE_2D);
	glCheckError();

	glBindTexture(GL_TEXTURE_2D, 0);
	
	texture.impl->id = id;
	return texture;
}

Texture TextureLoader::loadCubemap(const std::vector<std::string>& images)
{
	Texture texture;
	texture.impl = std::make_unique<TextureImpl>();
	texture.impl->type = TextureType_diffuse;

	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);
	glCheckError();

	for (unsigned int i = 0; i < images.size(); i++) {
		SDL_Surface* sdlTexture = IMG_Load(images[i].c_str());
		if (sdlTexture == NULL) {
			fprintf(stderr, "Could not load texture %s\n", images[i].c_str());
			glDeleteTextures(1, &id);
			texture.impl->id = 0;
			return texture;
		}

		unsigned int mode = imageColorMode(sdlTexture->format);
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
			mode, sdlTexture->w, sdlTexture->h, 0, mode, GL_UNSIGNED_BYTE, sdlTexture->pixels
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

	texture.impl->id = id;
	return texture;
}
