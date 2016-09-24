#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <noise/noise.h>

#include "TerrainPatch.h"

class Terrain
{
public:
	Terrain();
	Terrain(int seed);
	Terrain(unsigned patchSize, float stepSize, unsigned octaves, float baseFrequency, float persistence, int seed);

	void setPatchSize(unsigned patchSize);

	TerrainPatch generatePatch(int x, int y);
private:
	noise::module::Perlin noise;
	unsigned patchSize;
	float stepSize;
};