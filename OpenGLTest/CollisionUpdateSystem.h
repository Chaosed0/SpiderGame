#pragma once

#include "System.h"

class CollisionUpdateSystem : public System
{
public:
	CollisionUpdateSystem();
	void updateEntity(float dt, Entity& entity);
};