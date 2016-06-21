
#include "TerrainPatch.h"

static const glm::vec3 testColor(1 / 255.0f, 142 / 255.0f, 14 / 255.0f);
static const float textureTiling = 3.5f;

Model TerrainPatch::toModel(glm::vec2 origin, glm::vec3 scale)
{
	std::vector<Vertex> vertices;
	unsigned vi = 0;

	for (unsigned y = 0; y < this->size.y; y++) {
		for (unsigned x = 0; x < this->size.x; x++) {
			Vertex vertex;
			float val = this->terrain[y * this->size.x + x] * scale.y;
			vertex.position = glm::vec3(origin.x + (int)x * scale.x, val, origin.y + (int)y * scale.z);
			vertex.tintColor = testColor;
			vertex.texCoords = glm::vec2(fmod(vertex.position.x, textureTiling) / textureTiling, fmod(vertex.position.z, textureTiling) / textureTiling);
			vertices.emplace_back(std::move(vertex));
		}
	}

	for (unsigned y = 0; y < this->size.y; y++) {
		for (unsigned x = 0; x < this->size.x; x++) {
			Vertex& vertex = vertices[y * this->size.x + x];
			glm::vec3 sum;
			glm::vec3 left, right, up, down;
			if (x > 0) {
				Vertex& vLeft = vertices[y * this->size.x + (x - 1)];
				left = vLeft.position - vertex.position;
			}
			if (x < this->size.x - 1) {
				Vertex& vRight = vertices[y * this->size.x + (x + 1)];
				right = vRight.position - vertex.position;
			}
			if (y > 0) {
				Vertex& vUp = vertices[(y - 1) * this->size.x + x];
				up = vUp.position - vertex.position;
			}
			if (y < this->size.y - 1) {
				Vertex& vDown = vertices[(y + 1) * this->size.x + x];
				down = vDown.position - vertex.position;
			}

			if (left.x != 0.0f && up.z != 0.0f) {
				sum += glm::cross(up, left);
			}
			if (left.x != 0.0f && down.z != 0.0f) {
				sum += glm::cross(left, down);
			}
			if (right.x != 0.0f && up.z != 0.0f) {
				sum += glm::cross(right, up);
			}
			if (right.x != 0.0f && down.z != 0.0f) {
				sum += glm::cross(down, right);
			}

			vertex.normal = glm::normalize(sum);
		}
	}

	std::vector<GLuint> indices;
	for (unsigned y = 0; y < this->size.y - 1; y++) {
		for (unsigned x = 0; x < this->size.x - 1; x++) {
			int tli = y * this->size.x + x;
			int tri = tli + 1;
			int bli = (y + 1) * this->size.x + x;
			int bri = bli + 1;

			indices.push_back(tli);
			indices.push_back(bri);
			indices.push_back(bli);
			indices.push_back(tli);
			indices.push_back(tri);
			indices.push_back(bri);
		}
	}

	std::vector<Texture> textures(1);
	textures[0].loadFromFile("assets/img/terrain_shading.png");
	textures[0].type = TextureType_diffuse;

	return Model(std::vector<Mesh> { Mesh(vertices, indices, textures) });
}

TerrainPatchCollision TerrainPatch::getCollisionData(glm::vec2 origin, glm::vec3 scale)
{
	TerrainPatchCollision data;

	for (unsigned y = 0; y < this->size.y; y++) {
		for (unsigned x = 0; x < this->size.x; x++) {
			float val = this->terrain[y * this->size.x + x] * scale.y;
			data.vertices.push_back(origin.x + (int)x * scale.x);
			data.vertices.push_back(val);
			data.vertices.push_back(origin.y + (int)y * scale.z);
		}
	}

	for (unsigned y = 0; y < this->size.y - 1; y++) {
		for (unsigned x = 0; x < this->size.x - 1; x++) {
			int tli = y * this->size.x + x;
			int tri = tli + 1;
			int bli = (y + 1) * this->size.x + x;
			int bri = bli + 1;

			data.indices.push_back(tli);
			data.indices.push_back(bri);
			data.indices.push_back(bli);
			data.indices.push_back(tli);
			data.indices.push_back(tri);
			data.indices.push_back(bri);
		}
	}

	return data;
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
	for (int y = 0; y < surface->h; y++) {
		for (int x = 0; x < surface->pitch; x++) {
			float value = (this->terrain[y * this->size.x + x] - this->min) / (this->max - this->min);
			pixels[y * surface->pitch + x] = (unsigned char)(value * 255);
		}
	}
	SDL_SaveBMP(surface, file.c_str());
	SDL_FreeSurface(surface);
}
