#pragma once

#include "Framework/System.h"

class ExpiresSystem : public System
{
public:
	ExpiresSystem(World& world);
	virtual void updateEntity(float dt, eid_t entity);
};
