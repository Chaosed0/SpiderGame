#pragma once

#include "Framework/System.h"

#include <random>

class ShakeSystem : public System
{
public:
	ShakeSystem(World& world, std::default_random_engine& generator);
	virtual void updateEntity(float dt, eid_t entity);
private:
	std::default_random_engine& generator;
};
