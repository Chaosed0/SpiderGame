#pragma once

#include "Framework/System.h"

class CollisionUpdateSystem : public System
{
public:
	CollisionUpdateSystem(World& world);
	void updateEntity(float dt, eid_t entity);
};