
#include "Physics.h"

#include <algorithm>
#include <cassert>

static void bulletTickCallback(btDynamicsWorld* world, btScalar timeStep)
{
	Physics* physics = static_cast<Physics*>(world->getWorldUserInfo());
	physics->fixedUpdate(world, timeStep);
}

Physics::Physics(btDynamicsWorld* dynamicsWorld)
	: dynamicsWorld(dynamicsWorld),
	currentFrame(0)
{ }

void Physics::fixedUpdate(btDynamicsWorld* world, float timeStep)
{
	assert(dynamicsWorld == world);
	++this->currentFrame;

	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	for (int i = 0; i < numManifolds; i++) {
		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		const btCollisionObject* obA = contactManifold->getBody0();
		const btCollisionObject* obB = contactManifold->getBody1();

		if (obA->getUserPointer() == nullptr ||
			obB->getUserPointer() == nullptr)
		{
			continue;
		}

		eid_t e1 = *((eid_t*)obA->getUserPointer());
		eid_t e2 = *((eid_t*)obB->getUserPointer());
		eid_t emin = std::min(e1, e2);
		eid_t emax = std::max(e1, e2);

		PhysicsContact& contact = contacts[emin][emax];
		contact.lastContactFrame = currentFrame;
		contact.contactManifold = contactManifold;
	}
}

btPersistentManifold* Physics::getContact(eid_t e1, eid_t e2)
{
	eid_t emin = std::min(e1, e2);
	eid_t emax = std::max(e1, e2);
	auto i1 = contacts.find(emin);
	if (i1 == contacts.end()) {
		return nullptr;
	}

	auto i2 = i1->second.find(emax);
	if (i2 == i1->second.end()) {
		return nullptr;
	}

	PhysicsContact& contact = i2->second;
	if (contact.lastContactFrame != currentFrame) {
		return nullptr;
	}

	return contact.contactManifold;
}
