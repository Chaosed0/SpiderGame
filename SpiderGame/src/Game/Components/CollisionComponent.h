#pragma once

#include "Framework/Component.h"
#include "Framework/ComponentConstructor.h"

#include "Game/Extra/PrefabConstructionInfo.h"
#include "Game/Components/TransformComponent.h"
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
	virtual ComponentConstructorInfo construct(World& world, eid_t parent, void* userinfo) const
	{
		PrefabConstructionInfo* constructionInfo = (PrefabConstructionInfo*)userinfo;

		Transform initialTransform;
		if (constructionInfo != nullptr) {
			initialTransform = constructionInfo->initialTransform;

			if (parent != World::NullEntity) {
				TransformComponent* parentTransformComponent = world.getComponent<TransformComponent>(parent);
				if (parentTransformComponent != nullptr) {
					initialTransform.setParent(parentTransformComponent->data);

					if (this->controlsMovement) {
						printf("WARNING: Collision component controls movement, but is parented to entity with a transform component");
					}
				}
			}
		}

		btRigidBody::btRigidBodyConstructionInfo info(this->info);
		info.m_motionState = new btDefaultMotionState(Util::gameToBt(initialTransform));

		CollisionComponent* component = new CollisionComponent();
		btRigidBody* body = new btRigidBody(info);

		component->world = this->world;
		component->collisionObject = body;
		component->controlsMovement = controlsMovement;

		this->world->addRigidBody(body, group, mask);
		return ComponentConstructorInfo(component, typeid(CollisionComponent).hash_code());
	}

	virtual void finish(World& world, eid_t entity) {
		CollisionComponent* component = world.getComponent<CollisionComponent>(entity);
		component->collisionObject->setUserPointer(new eid_t(entity));
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