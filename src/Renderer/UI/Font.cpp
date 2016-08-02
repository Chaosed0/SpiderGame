
#include "Font.h"

#include <Windows.h>
#include <GL/glew.h>

#include <algorithm>
#include <sstream>

#include <SDL.h>

Font::Font()
	: texturePacker(glm::ivec2(64, 64))
{ }

Font::Font(const std::string& fontPath, int height)
	: Font()
{
	this->loadCharacters(fontPath, height);
}

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
	glBindTexture(GL_TEXTURE_2D, this->textureID);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, textureSize.x, textureSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, buffer);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Font::saveAtlasToFile(const std::string& file) const
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

Character Font::getCharacter(unsigned int i) const
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

unsigned Font::getTextureId() const
{
	return textureID;
}

glm::ivec2 Font::getTextureSize() const
{
	return texturePacker.getTextureSize();
}

Mesh Font::generateMesh(const std::string& text)
{
	unsigned x = 0;
	unsigned y = 0;
	size_t lineSize = text.size();
	float scale = 1.0f;
	glm::ivec2 textureSize = this->getTextureSize();

	// four verts per character
	std::vector<Vertex> vertices(4 * lineSize);

	// Six indices per quad
	std::vector<unsigned> indices(6 * lineSize);

	for (unsigned int i = 0; i < lineSize; i++) {
		Character ch = this->getCharacter(text[i]);

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;

		// Normalize texture coordinates
		float texLeft = ch.origin.x / (float)textureSize.x;
		float texRight = (ch.origin.x + ch.size.x) / (float)textureSize.x;
		float texBot = ch.origin.y / (float)textureSize.y;
		float texTop = (ch.origin.y + ch.size.y) / (float)textureSize.y;

		vertices[4 * i + 0].position = glm::vec3(xpos, ypos, 0.0f);
		vertices[4 * i + 0].texCoords = glm::vec2(texLeft, texTop);
		vertices[4 * i + 1].position = glm::vec3(xpos + w, ypos, 0.0f);
		vertices[4 * i + 1].texCoords = glm::vec2(texRight, texTop);
		vertices[4 * i + 2].position = glm::vec3(xpos, ypos + h, 0.0f);
		vertices[4 * i + 2].texCoords = glm::vec2(texLeft, texBot);
		vertices[4 * i + 3].position = glm::vec3(xpos + w, ypos + h, 0.0f);
		vertices[4 * i + 3].texCoords = glm::vec2(texRight, texBot);

		indices[6 * i + 0] = 4 * i + 0;
		indices[6 * i + 1] = 4 * i + 1;
		indices[6 * i + 2] = 4 * i + 2;
		indices[6 * i + 3] = 4 * i + 1;
		indices[6 * i + 4] = 4 * i + 3;
		indices[6 * i + 5] = 4 * i + 2;

		// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		x += (unsigned)((ch.advance >> 6) * scale); // Bitshift by 6 to get value in pixels (2^6 = 64)
	}

	return Mesh (vertices, indices, std::vector<Texture> { Texture(TextureType_diffuse, this->textureID) });
}
