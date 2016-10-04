#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

class HealthComponent : public Component
{
public:
	struct Data {
		Data(int health) : health(health), maxHealth(health) { }
		Data() : Data(100) { }
		int health;
		unsigned maxHealth;
	};
	Data data;
};

class HealthConstructor : public DefaultComponentConstructor<HealthComponent> {
	using DefaultComponentConstructor<HealthComponent>::DefaultComponentConstructor;
};
