#pragma once

#include "Framework/System.h"

class CollisionUpdateSystem : public System
{
public:
	CollisionUpdateSystem();
	void updateEntity(float dt, Entity& entity);
};