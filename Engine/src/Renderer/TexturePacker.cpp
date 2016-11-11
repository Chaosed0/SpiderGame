
#include "TexturePacker.h"

TexturePacker::TexturePacker()
	: TexturePacker(glm::ivec2(512, 512))
{ }

TexturePacker::TexturePacker(glm::ivec2 initialSize)
{
	resizeBuffer(initialSize);

	rootNode = std::make_unique<TextureNode>();
	rootNode->origin = glm::ivec2(0,0);
	rootNode->size = glm::ivec2(INT_MAX,INT_MAX);
	rootNode->empty = true;
	rootNode->left = nullptr;
	rootNode->right = nullptr;
}

void TexturePacker::resizeBuffer(const glm::ivec2& newSize)
{
	std::vector<unsigned char> newBuffer;
	newBuffer.resize(newSize.y*newSize.x);
	for (int y = 0; y < textureSize.y; y++) {
		for (int x = 0; x < textureSize.x; x++) {
			newBuffer[y * newSize.x + x] = buffer[y * textureSize.x + x];
		}
	}
	
	textureSize = newSize;
	buffer = std::move(newBuffer);
}

TexturePacker::TextureNode* TexturePacker::pack(TextureNode* node, const glm::ivec2& size)
{
	if (!node->empty) {
		// Filled, we have to be a leaf
		assert(!node->left && !node->right);
		return NULL;
	} else if (node->left && node->right) {
		// Non-leaf, try inserting to the left and then to the right
		assert(node->left && node->right);
		TextureNode* retval = pack(node->left.get(), size);
		if (retval != NULL) {
			return retval;
		}
		return pack(node->right.get(), size);
	} else {
		glm::ivec2 realSize(node->origin.x + node->size.x == INT_MAX ? textureSize.x - node->origin.x : node->size.x,
			node->origin.y + node->size.y == INT_MAX ? textureSize.y - node->origin.y : node->size.y);
		// Unfilled leaf - try to fill
		if (node->size.x == size.x && node->size.y == size.y) {
			// This is an interior cell which perfectly fits, just pack it
			node->empty = false;
			return node;
		} else if (realSize.x < size.x || realSize.y < size.y) {
			// Not big enough
			return NULL;
		} else {
			// Large enough - split
			TextureNode* left;
			TextureNode* right;
			int remainX = realSize.x - size.x;
			int remainY = realSize.y - size.y;

			bool verticalSplit = remainX < remainY;
			if (remainX == 0 && remainY == 0) {
				// This is an exterior cell which perfectly fits - split along the side which is closer to INT_MAX
				if (node->size.x > node->size.y) {
					verticalSplit = false;
				} else {
					verticalSplit = true;
				}
			}

			if (verticalSplit) {
				// Split vertically (left is top)
				left = new TextureNode(node->origin, glm::ivec2(node->size.x, size.y));
				right = new TextureNode(glm::ivec2(node->origin.x, node->origin.y + size.y),
										glm::ivec2(node->size.x, node->size.y - size.y));
			} else {
				// Split horizontally
				left = new TextureNode(node->origin, glm::ivec2(size.x, node->size.y));
				right = new TextureNode(glm::ivec2(node->origin.x + size.x, node->origin.y),
										glm::ivec2(node->size.x - size.x, node->size.y));
			}

			node->left = std::unique_ptr<TextureNode>(left);
			node->right = std::unique_ptr<TextureNode>(right);
			return pack(node->left.get(), size);
		}
	}
}

glm::ivec2 TexturePacker::packTexture(unsigned char* textureBuffer, const glm::ivec2& bufferSize)
{
	TextureNode* node = pack(rootNode.get(), bufferSize);
	if (node == NULL) {
		this->resizeBuffer(glm::ivec2(textureSize.x*2, textureSize.y*2));
		node = pack(rootNode.get(), bufferSize);
		assert(node != NULL);
	}

	assert(bufferSize.x == node->size.x);
	assert(bufferSize.y == node->size.y);

	for (int ly = 0; ly < bufferSize.y; ly++) {
		for (int lx = 0; lx < bufferSize.x; lx++) {
			int y = node->origin.y + ly;
			int x = node->origin.x + lx;
			buffer[y * textureSize.x + x] = textureBuffer[ly * bufferSize.x + lx];
		}
	}

	return node->origin;
}

glm::ivec2 TexturePacker::getTextureSize() const
{
	return textureSize;
}

const unsigned char* TexturePacker::getBuffer() const
{
	return buffer.data();
}
