#pragma once

#include "Framework/System.h"
#include "Framework/Prefab.h"

#include <random>

#include <glm/glm.hpp>

class World;
class btDynamicsWorld;

class SpawnerSystem : public System
{
public:
	SpawnerSystem(World& world, btDynamicsWorld* dynamicsWorld, std::default_random_engine& generator);
	virtual void updateEntity(float dt, eid_t entity);
private:
	btDynamicsWorld* dynamicsWorld;

	std::default_random_engine& generator;

	static const unsigned maximumSpawnRetries;
	static const unsigned maxSpiders;
};
