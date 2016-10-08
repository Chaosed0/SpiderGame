
#include "Framework/Event.h"

class ShotEvent : public Event
{
public:
	eid_t source;
	bool actuallyShot;
};

class ReloadStartEvent : public Event
{
public:
	eid_t source;
};

class ReloadEndEvent : public Event
{
	public:
	eid_t source;
};