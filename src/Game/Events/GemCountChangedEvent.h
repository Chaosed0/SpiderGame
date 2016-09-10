#pragma once

#include "Framework/World.h"
#include "Framework/Event.h"

class GemCountChangedEvent : public Event
{
public:
	eid_t source;
	unsigned oldGemCount;
	unsigned newGemCount;
};