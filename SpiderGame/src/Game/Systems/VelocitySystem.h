#pragma once

#include "Framework/System.h"

class VelocitySystem : public System
{
public:
	VelocitySystem(World& world);
	virtual void updateEntity(float dt, eid_t entity);
};
