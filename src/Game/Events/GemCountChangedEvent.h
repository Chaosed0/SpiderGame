#pragma once

#include "Framework/World.h"
#include "Framework/Event.h"

class GemCountChangedEvent : public Event
{
public:
	unsigned oldGemCount;
	unsigned newGemCount;
};