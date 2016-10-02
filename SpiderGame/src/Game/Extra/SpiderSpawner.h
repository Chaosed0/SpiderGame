#pragma once

#include "Renderer/Renderer.h"
#include "Renderer/ModelLoader.h"
#include "Sound/SoundManager.h"
#include "Framework/World.h"
#include "Environment/Room.h"

#include <btBulletDynamicsCommon.h>

class SpiderSpawner
{
public:
	SpiderSpawner(Renderer& renderer, SoundManager& soundManager, World& world, btDynamicsWorld* dynamicsWorld, ModelLoader& modelLoader, std::default_random_engine& generator, Shader& spiderShader, Room room, eid_t player);

	void update(float dt);
private:
	float startSpawnTime;
	float endSpawnTime;
	float difficultyRampTime;
	float minSpawnDistance;

	float spawnTimer;
	float difficultyRampTimer;

	Renderer& renderer;
	SoundManager& soundManager;
	World& world;
	btDynamicsWorld* dynamicsWorld;
	Room room;
	eid_t player;

	std::vector<AudioClip> spiderSounds;
	AudioClip spiderDeathSound;

	Renderer::ModelHandle spiderModelHandle;
	Shader& spiderShader;

	std::default_random_engine& generator;
	std::uniform_real_distribution<float> scaleRand;
	std::uniform_int_distribution<int> roomRand;

	eid_t makeSpider(glm::vec3 position);

	static const float defaultStartSpawnTime;
	static const float defaultEndSpawnTime;
	static const float defaultDifficultyRampTime;
	static const float defaultMinSpawnDistance;
	static const int maximumSpawnRetries;
	static const int maxSpiders;
};