#pragma once

#include "Framework/Component.h"

class HealthComponent : public Component
{
public:
	HealthComponent() : health(100), maxHealth(100) { }
	int health;
	unsigned maxHealth;
};