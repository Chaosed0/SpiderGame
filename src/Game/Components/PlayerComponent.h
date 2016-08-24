#pragma once

#include "Framework/Component.h"
#include "Framework/World.h"

class PlayerComponent : public Component
{
public:
	PlayerComponent()
		: shooting(false),
		maxShotDistance(100.0f), shotCooldown(1/30.0f), shotTimer(0.0f), shotDamage(10),
		gemCount(0),
		lastFacedEntity(World::NullEntity), camera(World::NullEntity) { }

	bool shooting;
	float shotCooldown;
	float shotTimer;
	float maxShotDistance;
	unsigned shotDamage;
	unsigned gemCount;
	eid_t lastFacedEntity;

	eid_t camera;
};
