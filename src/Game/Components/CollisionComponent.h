#pragma once

#include "Framework/Component.h"

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <memory>

struct CollisionComponent : public Component
{
	CollisionComponent() : world(nullptr), collisionObject(nullptr), controlsMovement(true) { }
	~CollisionComponent()
	{
		if (world != nullptr) {
			world->removeCollisionObject(this->collisionObject);
		}

		eid_t* eid = (eid_t*)collisionObject->getUserPointer();
		if (eid != nullptr) {
			delete eid;
		}
	}

	btDynamicsWorld* world;
	btCollisionObject* collisionObject;
	bool controlsMovement;
};