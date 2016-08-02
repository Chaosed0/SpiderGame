
#include "Font.h"

#include <Windows.h>
#include <GL/glew.h>

#include <algorithm>
#include <sstream>

#include <SDL.h>

Font::Font()
	: texturePacker(glm::ivec2(64, 64))
{ }

void Font::loadCharacters(const std::string& fontPath, int height)
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "ERROR::FREETYPE: Could not init FreeType Library");
		return;
	}

	FT_Face face;
	if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
		fprintf(stderr, "ERROR::FREETYPE: Failed to load font %s\n", fontPath.c_str());
		return;
	}

	characters.clear();

	FT_Set_Pixel_Sizes(face, 0, height);

	for (int i = 0; i < 128; i++)
	{
		char c = (char)i;
		// Load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			fprintf(stderr, "ERROR::FREETYTPE: Failed to load Glyph %c", c);
			continue;
		}

		// Now store character for later use
		Character character = {
			glm::ivec2(0,0),
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x,
		};
		characters.push_back(character);

		this->packCharacter(characters[i], face->glyph->bitmap);
	}

	glm::ivec2 textureSize = texturePacker.getTextureSize();
	const unsigned char* buffer = texturePacker.getBuffer();

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// Give OpenGL the data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &this->textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, textureSize.x, textureSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Font::saveAtlasToFile(const std::string& file)
{
	glm::ivec2 textureSize = texturePacker.getTextureSize();
	const unsigned char* buffer = texturePacker.getBuffer();

	SDL_Surface* surface = SDL_CreateRGBSurface(0, textureSize.x, textureSize.y, 8,0,0,0,0);
	SDL_Color colors[256];
	for (int i = 0; i < 256; i++) {
		colors[i].r = colors[i].b = colors[i].g = i;
		colors[i].a = 255;
	}
	SDL_SetPaletteColors(surface->format->palette, colors, 0, 256);
	surface->pixels = (void*)buffer;

	SDL_SaveBMP(surface, file.c_str());
	SDL_FreeSurface(surface);
}

Character Font::getCharacter(unsigned int i)
{
	if (i < characters.size()) {
		return characters[i];
	}
	return Character { glm::ivec2(0,0), glm::ivec2(0,0), glm::ivec2(0,0), 0 };
}

void Font::packCharacter(Character& character, const FT_Bitmap& bitmap)
{
	if (character.size.x == 0 || character.size.y == 0) {
		return;
	}

	glm::ivec2 origin = texturePacker.packTexture(bitmap.buffer, glm::ivec2(bitmap.pitch, bitmap.rows));
	character.origin = origin;
}

unsigned Font::getTextureId()
{
	return textureID;
}

glm::ivec2 Font::getTextureSize()
{
	return texturePacker.getTextureSize();
}
