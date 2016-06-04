#pragma once

#include "Framework/System.h"

class TestMovementSystem : public System
{
public:
	TestMovementSystem();
	void updateEntity(float dt, Entity& entity);
private:
	float totalTime;
};