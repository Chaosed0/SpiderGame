#pragma once

#include "Framework/World.h"
#include "Framework/Event.h"

class BulletCountChangedEvent : public Event
{
public:
	eid_t source;
	unsigned oldBulletCount;
	unsigned newBulletCount;
};