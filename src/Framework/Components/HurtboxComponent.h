#pragma once

#include "Framework/World.h"
#include "Framework/Component.h"

#include <vector>

class HurtboxComponent : public Component
{
public:
	HurtboxComponent() : damage(10.0f) { }
	std::vector<eid_t> collidedEntities;
	float damage;
};
