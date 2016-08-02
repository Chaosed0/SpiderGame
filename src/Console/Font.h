#pragma once

#include <glm/glm.hpp>

#include <ft2build.h>
#include <freetype/freetype.h>

#include <vector>
#include <memory>

#include "Renderer/TexturePacker.h"

struct Character {
	glm::ivec2 origin;     // Position of glyph on texture atlas
	glm::ivec2 size;       // Size of glyph
	glm::ivec2 bearing;    // Offset from baseline to left/top of glyph
	FT_Pos     advance;    // Offset to advance to next glyph
};

class Font
{
public:
	Font();
	
	void loadCharacters(const std::string& fontPath, int height);
	unsigned getTextureId();
	glm::ivec2 getTextureSize();
	Character getCharacter(unsigned int i);
	void saveAtlasToFile(const std::string& file);
private:
	unsigned textureID;
	std::vector<Character> characters;
	TexturePacker texturePacker;

	void packCharacter(Character& character, const FT_Bitmap& bitmap);
};