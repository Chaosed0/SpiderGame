#pragma once

#include "Framework/System.h"
#include "Framework/EventManager.h"
#include "Renderer/Renderer.h"
#include "Game/Components/PlayerComponent.h"

#include <random>

class btDynamicsWorld;

class ShootingSystem : public System
{
public:
	ShootingSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer, EventManager& eventManager, std::default_random_engine& generator);
	virtual void updateEntity(float dt, eid_t entity);
private:
	btDynamicsWorld* dynamicsWorld;
	Renderer& renderer;
	EventManager& eventManager;
	std::default_random_engine& generator;

	std::uniform_real_distribution<float> randomAngleDistribution;

	eid_t createTracer(PlayerComponent* playerComponent, float expiryTime, float speed);
	eid_t createMuzzleFlash(PlayerComponent* playerComponent, float expiryTime);
};