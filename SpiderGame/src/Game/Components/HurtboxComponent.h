#pragma once

#include "Framework/World.h"
#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

#include <vector>

class HurtboxComponent : public Component
{
public:
	struct Data {
		Data() : damage(10) { }
		int damage;
	};
	Data data;
	std::vector<eid_t> collidedEntities;
};

class HurtboxConstructor : public DefaultComponentConstructor<HurtboxComponent> {
	using DefaultComponentConstructor<HurtboxComponent>::DefaultComponentConstructor;
};
