#pragma once

#include "Framework/World.h"
#include "Framework/Event.h"

class DamageEvent : public Event
{
public:
	eid_t aggressor;
	eid_t victim;
	unsigned damage;
};