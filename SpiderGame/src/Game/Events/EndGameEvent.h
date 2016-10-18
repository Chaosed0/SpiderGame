#pragma once

#include "Framework/Event.h"

class VictoryEvent : public Event
{ };

class RestEndEvent : public Event
{ };

class AllGemsCollectedEvent : public Event
{
public:
	eid_t source;
};
