
#include "Font.h"

#include <algorithm>
#include <sstream>

#include <SDL.h>

Font::Font()
	: textureSize(64,64)
{
	buffer.resize(textureSize.y*textureSize.x);
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

	rootNode = std::make_unique<Node>();
	rootNode->origin = glm::ivec2(0,0);
	rootNode->size = glm::ivec2(INT_MAX,INT_MAX);
	rootNode->empty = true;
	rootNode->left = nullptr;
	rootNode->right = nullptr;

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

	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// Give OpenGL the data
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &this->textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, textureSize.x, textureSize.y, 0, GL_RED, GL_UNSIGNED_BYTE, buffer.data());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Font::saveAtlasToFile(const std::string& file)
{
	SDL_Surface* surface = SDL_CreateRGBSurface(0, textureSize.x, textureSize.y, 8,0,0,0,0);
	SDL_Color colors[256];
	for (int i = 0; i < 256; i++) {
		colors[i].r = colors[i].b = colors[i].g = i;
		colors[i].a = 255;
	}
	SDL_SetPaletteColors(surface->format->palette, colors, 0, 256);
	surface->pixels = (void*)buffer.data();

	SDL_SaveBMP(surface, file.c_str());
	SDL_FreeSurface(surface);
}

GLuint Font::getTextureId()
{
	return textureID;
}

Character Font::getCharacter(unsigned int i)
{
	if (i < characters.size()) {
		return characters[i];
	}
	return Character { glm::ivec2(0,0), glm::ivec2(0,0), glm::ivec2(0,0), 0 };
}

glm::ivec2 Font::getTextureSize()
{
	return textureSize;
}

void Font::packCharacter(Character& character, const FT_Bitmap& bitmap)
{
	if (character.size.x == 0 || character.size.y == 0) {
		return;
	}

	Node* node = pack(rootNode.get(), character.size);
	if (node == NULL) {
		this->resizeBuffer(glm::ivec2(textureSize.x*2, textureSize.y*2));
		node = pack(rootNode.get(), character.size);
		assert(node != NULL);
	}

	assert(character.size.x == node->size.x);
	assert(character.size.y == node->size.y);
	character.origin = node->origin;

	for (unsigned int ly = 0; ly < bitmap.rows; ly++) {
		for (unsigned int lx = 0; lx < bitmap.width; lx++) {
			int y = character.origin.y + ly;
			int x = character.origin.x + lx;
			buffer[y * textureSize.x + x] = bitmap.buffer[ly * std::abs(bitmap.pitch) + lx];
		}
	}
}

void Font::resizeBuffer(const glm::ivec2 newSize)
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

Node* Font::pack(Node* node, const glm::ivec2& size)
{
	if (!node->empty) {
		// Filled, we have to be a leaf
		assert(!node->left && !node->right);
		return NULL;
	} else if (!node->left && !node->right) {
		glm::ivec2 realSize(node->origin.x + node->size.x == INT_MAX ? textureSize.x - node->origin.x : node->size.x,
			node->origin.y + node->size.y == INT_MAX ? textureSize.y - node->origin.y : node->size.y);
		// Unfilled leaf - try to fill
		if (node->size.x == size.x && node->size.y == size.y) {
			// Perfect - just pack
			node->empty = false;
			return node;
		} else if (realSize.x < size.x || realSize.y < size.y) {
			// Not big enough
			return NULL;
		} else {
			// Large enough - split
			std::unique_ptr<Node> left, right;
			int remainX = realSize.x - size.x;
			int remainY = realSize.y - size.y;

			bool verticalSplit = remainX < remainY;
			if (remainX == 0 && remainY == 0) {
				// Edge case - we are at a realSize boundary, split along the side which is closer to INT_MAX
				if (node->size.x > node->size.y) {
					verticalSplit = false;
				} else {
					verticalSplit = true;
				}
			}

			if (verticalSplit) {
				// Split vertically (left is top)
				left = std::make_unique<Node>(node->origin, glm::ivec2(node->size.x, size.y));
				right = std::make_unique<Node>(glm::ivec2(node->origin.x, node->origin.y + size.y), glm::ivec2(node->size.x, node->size.y - size.y));
			} else {
				// Split horizontally
				left = std::make_unique<Node>(node->origin, glm::ivec2(size.x, node->size.y));
				right = std::make_unique<Node>(glm::ivec2(node->origin.x + size.x, node->origin.y), glm::ivec2(node->size.x - size.x, node->size.y));
			}

			node->left = std::move(left);
			node->right = std::move(right);
			return pack(node->left.get(), size);
		}
	} else {
		// Non-leaf, try inserting to the left and then to the right
		assert(node->left && node->right);
		Node* retval = pack(node->left.get(), size);
		if (retval != NULL) {
			return retval;
		}
		return pack(node->right.get(), size);
	}
}
