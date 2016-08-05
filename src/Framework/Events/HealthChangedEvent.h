#pragma once

#include "Framework/World.h"
#include "Framework/Event.h"

class HealthChangedEvent : public Event
{
public:
	eid_t entity;
	unsigned newHealth;
	unsigned healthChange;
};