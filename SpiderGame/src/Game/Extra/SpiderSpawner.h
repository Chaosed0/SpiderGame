#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/ModelLoader.h"
#include "Sound/SoundManager.h"
#include "Framework/World.h"
#include "Framework/Prefab.h"
#include "Environment/Room.h"

#include <btBulletDynamicsCommon.h>

class SpiderSpawner
{
public:
	SpiderSpawner(World& world, btDynamicsWorld* dynamicsWorld, Prefab spiderPrefab, Room room, std::default_random_engine& generator);

	void update(float dt);
private:
	float startSpawnTime;
	float endSpawnTime;
	float difficultyRampTime;
	float minSpawnDistance;

	float spawnTimer;
	float difficultyRampTimer;

	World& world;
	btDynamicsWorld* dynamicsWorld;
	Prefab spiderPrefab;
	Room room;

	std::default_random_engine& generator;
	std::uniform_real_distribution<float> scaleRand;
	std::uniform_int_distribution<int> roomRand;

	eid_t makeSpider(const glm::vec3& position);

	static const float defaultStartSpawnTime;
	static const float defaultEndSpawnTime;
	static const float defaultDifficultyRampTime;
	static const float defaultMinSpawnDistance;
	static const int maximumSpawnRetries;
	static const int maxSpiders;
};