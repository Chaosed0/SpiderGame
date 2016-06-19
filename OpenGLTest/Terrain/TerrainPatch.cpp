
#include "TerrainPatch.h"

Model TerrainPatch::toModel(glm::vec2 origin, glm::vec3 scale)
{
	std::vector<Vertex> vertices;
	vertices.resize(this->size.y * this->size.x * 6);
	unsigned vi = 0;

	for (unsigned y = 0; y < this->size.y - 1; y++) {
		for (unsigned x = 0; x < this->size.x - 1; x++) {
			// Make one quad
			unsigned i = y * this->size.x + x;
			float tlv = this->terrain[y * this->size.x + x] * scale.y;
			float trv = this->terrain[y * this->size.x + (x+1)] * scale.y;
			float blv = this->terrain[(y+1) * this->size.x + x] * scale.y;
			float brv = this->terrain[(y+1) * this->size.x + (x+1)] * scale.y;

			glm::vec3 tlp = glm::vec3(origin.x + x * scale.x, tlv, origin.y + y * scale.z);
			glm::vec3 trp = glm::vec3(origin.x + (x+1) * scale.x, trv, origin.y + y * scale.z);
			glm::vec3 blp = glm::vec3(origin.x + x * scale.x, blv, origin.y + (y+1) * scale.z);
			glm::vec3 brp = glm::vec3(origin.x + (x+1) * scale.x, brv, origin.y + (y+1) * scale.z);

			glm::vec3 n1 = glm::cross(tlp - blp, brp - blp);
			glm::vec3 n2 = glm::cross(tlp - trp, brp - trp);

			vertices[vi + 0].position = tlp;
			vertices[vi + 0].normal = n1;
			vertices[vi + 0].texCoords = glm::vec2(0.0f, 0.0f);

			vertices[vi + 1].position = brp;
			vertices[vi + 1].normal = n1;
			vertices[vi + 1].texCoords = glm::vec2(1.0f, 1.0f);
			
			vertices[vi + 2].position = blp;
			vertices[vi + 2].normal = n1;
			vertices[vi + 2].texCoords = glm::vec2(0.0f, 1.0f);

			vertices[vi + 3].position = tlp;
			vertices[vi + 3].normal = n2;
			vertices[vi + 3].texCoords = glm::vec2(0.0f, 0.0f);

			vertices[vi + 4].position = trp;
			vertices[vi + 4].normal = n2;
			vertices[vi + 4].texCoords = glm::vec2(1.0f, 0.0f);

			vertices[vi + 5].position = brp;
			vertices[vi + 5].normal = n2;
			vertices[vi + 5].texCoords = glm::vec2(1.0f, 1.0f);

			vi += 6;
		}
	}

	std::vector<GLuint> indices(vertices.size());
	for (unsigned i = 0; i < indices.size(); i++) {
		indices[i] = i;
	}

	std::vector<Texture> textures(1);
	textures[0].loadFromFile("assets/img/test.png");
	textures[0].type = TextureType_diffuse;

	return Model(std::vector<Mesh> { Mesh(vertices, indices, textures) });
}

void TerrainPatch::SaveHeightmapToFile(const std::string& file)
{
	SDL_Surface* surface = SDL_CreateRGBSurface(0, this->size.x, this->size.y, 8, 0, 0, 0, 0);

	SDL_Color colors[256];
	for (int i = 0; i < 256; i++) {
		colors[i].r = colors[i].b = colors[i].g = i;
		colors[i].a = 255;
	}
	SDL_SetPaletteColors(surface->format->palette, colors, 0, 256);

	unsigned char* pixels = (unsigned char*)surface->pixels;
	for (unsigned y = 0; y < surface->h; y++) {
		for (unsigned x = 0; x < surface->pitch; x++) {
			float value = (this->terrain[y * this->size.x + x] - this->min) / (this->max - this->min);
			pixels[y * surface->pitch + x] = (unsigned char)(value * 255);
		}
	}
	SDL_SaveBMP(surface, file.c_str());
	SDL_FreeSurface(surface);
}
