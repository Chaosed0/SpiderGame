
#include "CollisionResponder.h"

CollisionResponder::CollisionResponder(World& world)
	: world(world)
{ }

void CollisionResponder::handleCollisionResponse(eid_t e1, eid_t e2, btPersistentManifold* contactManifold, CollisionResponseType type)
{
	ComponentBitmask components1 = world.getEntityBitmask(e1);
	ComponentBitmask components2 = world.getEntityBitmask(e2);
	bool handle = false;

	if (components1.hasComponents(requiredComponents1) &&
		components2.hasComponents(requiredComponents2))
	{
		handle = true;
	}
	else if (components2.hasComponents(requiredComponents1) && 
		components1.hasComponents(requiredComponents2))
	{
		handle = true;
		eid_t tmp = e2;
		e2 = e1;
		e1 = tmp;
	}

	if (handle) {
		if (type == COLLISIONRESPONSETYPE_BEGAN) {
			this->collisionBegan(e1, e2, contactManifold);
		} else if (type == COLLISIONRESPONSETYPE_ENDED) {
			this->collisionEnded(e1, e2, contactManifold);
		}
	}
}
