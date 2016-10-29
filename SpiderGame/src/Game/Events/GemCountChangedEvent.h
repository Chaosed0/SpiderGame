#pragma once

#include "Framework/World.h"
#include "Framework/Event.h"

class GemCountChangedEvent : public Event
{
public:
	eid_t source;
};

class GemLightOnEvent : public Event
{
public:
	eid_t source;
};