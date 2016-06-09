
#include "Font.h"

#include <algorithm>
#include <sstream>

#include <SDL.h>

Font::Font()
	: textureSize(256,256)
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
	}

	rootNode = std::make_unique<Node>();
	rootNode->origin = glm::ivec2(0,0);
	rootNode->size = glm::ivec2(INT_MAX,INT_MAX);
	rootNode->empty = true;
	rootNode->left = nullptr;
	rootNode->right = nullptr;

	for (unsigned int i = 0; i < characters.size(); i++) {
		if (characters[i].size.x == 0 || characters[i].size.y == 0) {
			continue;
		}

		Node* node = pack(rootNode.get(), characters[i].size);
		if (node == NULL) {
			textureSize.x *= 2;
			textureSize.y *= 2;
			node = pack(rootNode.get(), characters[i].size);
			assert(node != NULL);
		}

		assert(characters[i].size.x == node->size.x);
		assert(characters[i].size.y == node->size.y);
		characters[i].origin = node->origin;
	}

	SDL_Surface* fontSurface = SDL_CreateRGBSurface(0, textureSize.x, textureSize.y, 8, 0, 0, 0, 0);
	SDL_Color colors[256];
	for (int i = 0; i < 256; i++) {
		colors[i].r = colors[i].g = colors[i].b = i;
	}

	SDL_SetPaletteColors(fontSurface->format->palette, colors, 0, 256);
	for (unsigned int i = 0; i < characters.size(); i++) {
		if (characters[i].size.x == 0 || characters[i].size.y == 0) {
			continue;
		}

		char c = (char)i;
		// Load character glyph
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			fprintf(stderr, "ERROR::FREETYTPE: Failed to load Glyph %c", c);
			continue;
		}

		SDL_Surface* characterSurface = SDL_CreateRGBSurface(0, characters[i].size.x, characters[i].size.y, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors(characterSurface->format->palette, colors, 0, 256);

		uint8_t* pixels = (uint8_t*)characterSurface->pixels;
		for (int y = 0; y < characterSurface->h; y++) {
			for (int x = 0; x < characterSurface->pitch; x++) {
				pixels[y * characterSurface->pitch + x] = face->glyph->bitmap.buffer[y * face->glyph->bitmap.pitch + x];
			}
		}

		std::stringstream sstr;
		sstr << "test/" << i << ".bmp";
		SDL_SaveBMP(characterSurface, sstr.str().c_str());

		SDL_Rect target { characters[i].origin.x, characters[i].origin.y, characters[i].size.x, characters[i].size.y };
		SDL_Rect origin { 0, 0, characters[i].size.x, characters[i].size.y };
		SDL_BlitSurface(characterSurface, &origin, fontSurface, &target);
	}
	SDL_SaveBMP(fontSurface, "kek.bmp");

	FT_Done_Face(face);
	FT_Done_FreeType(ft);
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
			float remainX = realSize.x - size.x;
			float remainY = realSize.y - size.y;

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
