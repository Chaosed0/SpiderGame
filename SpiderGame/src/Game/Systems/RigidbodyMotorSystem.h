#pragma once

#include "Framework/System.h"

class RigidbodyMotorSystem : public System
{
public:
	RigidbodyMotorSystem(World& world);
	void updateEntity(float dt, eid_t entity);
};