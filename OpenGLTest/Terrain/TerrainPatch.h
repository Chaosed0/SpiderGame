#pragma once

#include "Renderer/Model.h"

struct TerrainPatch
{
	std::vector<float> terrain;
	glm::ivec2 size;
	float max, min;

	Model toModel(glm::vec2 origin, glm::vec3 scale);
	void SaveHeightmapToFile(const std::string& file);
};
