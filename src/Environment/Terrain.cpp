
#include "Terrain.h"

#include <algorithm>

Terrain::Terrain()
{
	patchSize = 513;
	stepSize = 0.001f;
}

Terrain::Terrain(int seed)
{
	noise.SetSeed(seed);
}

Terrain::Terrain(unsigned patchSize, float stepSize, unsigned octaves, float baseFrequency, float persistence, int seed)
	: patchSize(patchSize),
	stepSize(stepSize)
{
	noise.SetOctaveCount(octaves);
	noise.SetFrequency(baseFrequency);
	noise.SetPersistence(persistence);
	noise.SetSeed(seed);
}

TerrainPatch Terrain::generatePatch(int x, int y)
{
	glm::vec2 start(x * (int)(patchSize-1) * stepSize, y * (int)(patchSize-1) * stepSize);
	TerrainPatch patch;
	patch.size.x = patchSize;
	patch.size.y = patchSize;
	patch.terrain.resize(patchSize*patchSize);
	patch.min = FLT_MAX;
	patch.max = FLT_MIN;

	for (unsigned y = 0; y < patchSize; y++) {
		for (unsigned x = 0; x < patchSize; x++) {
			float value = (float)noise.GetValue(start.x + x * stepSize, start.y + y * stepSize, 0.0f);
			patch.terrain[y * patchSize + x] = value;
			patch.min = std::fmin(value, patch.min);
			patch.max = std::fmax(value, patch.max);
		}
	}

	return patch;
}

void Terrain::setPatchSize(unsigned patchSize)
{
	this->patchSize = patchSize;
}
