#pragma once

#include "Framework/System.h"

class RigidbodyMotorSystem : public System
{
public:
	RigidbodyMotorSystem();
	void updateEntity(float dt, Entity& entity);
};