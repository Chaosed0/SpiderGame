#pragma once

#include "Framework/World.h"

#include <btBulletCollisionCommon.h>

enum CollisionResponseType
{
	COLLISIONRESPONSETYPE_BEGAN,
	COLLISIONRESPONSETYPE_ENDED
};

class CollisionResponder
{
public:
	CollisionResponder(World& world);

	void handleCollisionResponse(eid_t e1, eid_t e2, btPersistentManifold* contactManifold, CollisionResponseType type);

	virtual void collisionBegan(eid_t e1, eid_t e2, btPersistentManifold* contactManifold) = 0;
	virtual void collisionEnded(eid_t e1, eid_t e2, btPersistentManifold* contactManifold) = 0;

protected:
	template <class T>
	void requireOne();

	template <class T>
	void requireTwo();

	World& world;
private:
	ComponentBitmask requiredComponents1;
	ComponentBitmask requiredComponents2;
};

template <class T>
void CollisionResponder::requireOne()
{
	cid_t cid = world.getComponentId<T>();
	requiredComponents1.setBit(cid, true);
}

template <class T>
void CollisionResponder::requireTwo()
{
	cid_t cid = world.getComponentId<T>();
	requiredComponents2.setBit(cid, true);
}
