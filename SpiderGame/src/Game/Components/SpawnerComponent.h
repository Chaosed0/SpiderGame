#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

#include <glm/glm.hpp>

class SpawnerComponent : public Component
{
public:
	SpawnerComponent() : spawnTimer(0.0f), difficultyRampTimer(0.0f) { }

	struct Data
	{
		Data() : startSpawnTime(8.0f), endSpawnTime(3.0f), difficultyRampTime(60.0f), minSpawnDistance(7.0f) { }
		float startSpawnTime;
		float endSpawnTime;
		float difficultyRampTime;
		float minSpawnDistance;

		Prefab prefab;
		std::vector<glm::vec3> candidatePositions;
	};

	Data data;
	float spawnTimer;
	float difficultyRampTimer;
};

class SpawnerConstructor : public DefaultComponentConstructor<SpawnerComponent> {
	using DefaultComponentConstructor<SpawnerComponent>::DefaultComponentConstructor;
};