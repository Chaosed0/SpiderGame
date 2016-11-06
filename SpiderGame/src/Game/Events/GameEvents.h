#pragma once

#include "Framework/Event.h"

class RestartEvent : public Event
{
public:
	eid_t source;
};
