#pragma once

#include "Framework/Event.h"

class VictorySequenceStartedEvent : public Event
{ };

class VictorySequenceEndedEvent : public Event
{ };

class RestEndEvent : public Event
{ };

class AllGemsCollectedEvent : public Event
{
public:
	eid_t source;
};
