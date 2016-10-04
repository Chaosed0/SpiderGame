#pragma once

#include "Framework/Component.h"
#include "Framework/ComponentConstructor.h"
#include "Util.h"

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

class CollisionConstructor : public ComponentConstructor
{
public:
	CollisionConstructor(btDynamicsWorld* world, const btRigidBody::btRigidBodyConstructionInfo& info, int group = CollisionGroupDefault, int mask = CollisionGroupAll, bool controlsMovement = true)
		: world(world), info(info), group(group), mask(mask), controlsMovement(controlsMovement) { }
	virtual ComponentConstructorInfo construct() const
	{
		CollisionComponent* component = new CollisionComponent();
		btRigidBody* body = new btRigidBody(info);
		component->world = world;
		component->collisionObject = body;
		component->controlsMovement = controlsMovement;
		world->addRigidBody(body, group, mask);
		return ComponentConstructorInfo(component, typeid(CollisionComponent).hash_code());
	}

	void* operator new(size_t size) { return _mm_malloc(size, 16); }
	void operator delete(void* p) { _mm_free(p); }
private:
	btDynamicsWorld* world;
	btRigidBody::btRigidBodyConstructionInfo info;
	short group;
	short mask;
	bool controlsMovement;
};