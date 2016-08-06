#pragma once

#include "Framework/World.h"
#include "Framework/Event.h"

class DamageEvent : public Event
{
public:
	eid_t source;
	unsigned damage;
};