#pragma once

#include "Framework/World.h"
#include "Framework/Event.h"

class HealthChangedEvent : public Event
{
public:
	int newHealth;
	int healthChange;
};