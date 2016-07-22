#pragma once

#include <glm/glm.hpp>

#include <ft2build.h>
#include <freetype/freetype.h>

#include <vector>
#include <memory>

struct Character {
	glm::ivec2 origin;     // Position of glyph on texture atlas
	glm::ivec2 size;       // Size of glyph
	glm::ivec2 bearing;    // Offset from baseline to left/top of glyph
	FT_Pos     advance;    // Offset to advance to next glyph
};

struct Node
{
	Node() {}
	Node(glm::ivec2 origin, glm::ivec2 size)
		: origin(origin), size(size)
	{
		empty = true;
	}
	glm::ivec2 origin;
	glm::ivec2 size;
	bool empty;
	std::unique_ptr<Node> left;
	std::unique_ptr<Node> right;
};

class Font
{
public:
	Font();
	void loadCharacters(const std::string& fontPath, int height);
	unsigned getTextureId();
	Character getCharacter(unsigned int i);
	glm::ivec2 getTextureSize();
private:
	unsigned textureID;
	std::vector<Character> characters;
	std::unique_ptr<Node> rootNode;
	std::vector<unsigned char> buffer;
	glm::ivec2 textureSize;

	void packCharacter(Character& character, const FT_Bitmap& bitmap);
	void resizeBuffer(const glm::ivec2 newSize);
	void saveAtlasToFile(const std::string& file);
	Node* pack(Node* node, const glm::ivec2& size);
};