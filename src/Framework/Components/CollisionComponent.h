#pragma once

#include "Framework/Component.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <memory>

struct CollisionComponent : public Component
{
	CollisionComponent() : world(nullptr), body(nullptr) { }
	~CollisionComponent()
	{
		if (world != nullptr) {
			world->removeRigidBody(body);
		}

		eid_t* eid = (eid_t*)body->getUserPointer();
		if (eid != nullptr) {
			delete eid;
		}
	}

	btDynamicsWorld* world;
	btRigidBody* body;
};