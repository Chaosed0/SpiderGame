#pragma once

#include <GL/glew.h>
#include <Windows.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <glm/glm.hpp>

#include <ft2build.h>
#include <freetype/freetype.h>

#include <vector>
#include <memory>

struct Character {
	glm::ivec2 origin;     // Size of glyph
	glm::ivec2 size;       // Size of glyph
	glm::ivec2 bearing;    // Offset from baseline to left/top of glyph
	FT_Pos     advance;    // Offset to advance to next glyph
	unsigned char* buffer;
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
private:
	GLuint textureID;
	std::vector<Character> characters;
	std::unique_ptr<Node> rootNode;
	glm::ivec2 textureSize;

	Node* pack(Node* node, const glm::ivec2& size);
};