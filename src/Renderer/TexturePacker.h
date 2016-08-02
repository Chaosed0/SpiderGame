#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <memory>

class TexturePacker
{
public:
	TexturePacker();
	TexturePacker(glm::ivec2 initialSize);

	glm::ivec2 packTexture(unsigned char* textureBuffer, const glm::ivec2& bufferSize);

	glm::ivec2 getTextureSize();
	const unsigned char* getBuffer();
private:
	struct TextureNode
	{
		TextureNode() {}
		TextureNode(glm::ivec2 origin, glm::ivec2 size)
			: origin(origin), size(size) { empty = true; }
		glm::ivec2 origin;
		glm::ivec2 size;
		bool empty;
		std::unique_ptr<TextureNode> left;
		std::unique_ptr<TextureNode> right;
	};

	TextureNode* pack(TextureNode* node, const glm::ivec2& size);
	void resizeBuffer(const glm::ivec2& newSize);

	std::unique_ptr<TextureNode> rootNode;
	std::vector<unsigned char> buffer;
	glm::ivec2 textureSize;
};