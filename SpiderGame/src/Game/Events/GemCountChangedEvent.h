#pragma once

#include "Framework/World.h"
#include "Framework/Event.h"

enum GemColor {
	GemColor_Red = 0,
	GemColor_Green,
	GemColor_Blue,
	GemColor_Unknown
};

class GemCountChangedEvent : public Event
{
public:
	eid_t source;
	GemColor color;
};