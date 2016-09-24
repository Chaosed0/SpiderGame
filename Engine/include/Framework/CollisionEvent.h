#pragma once

#include "Framework/Event.h"
#include "Framework/Physics.h"

class CollisionEvent : public Event
{
public:
	eid_t e1;
	eid_t e2;
	btPersistentManifold* collisionManifold;
	CollisionResponseType type;
};