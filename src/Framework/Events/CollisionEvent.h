#pragma once

#include "Framework/Event.h"
#include "Framework/Physics.h"

class CollisionEvent : public Event
{
public:
	eid_t collidedEntity;
	btPersistentManifold* collisionManifold;
	CollisionResponseType type;
};