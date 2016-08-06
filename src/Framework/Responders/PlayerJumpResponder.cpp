
#include "PlayerJumpResponder.h"

#include "Framework/Physics.h"
#include "Framework/World.h"
#include "Framework/EventManager.h"

#include "Framework/Components/RigidbodyMotorComponent.h"
#include "Framework/Components/PlayerComponent.h"
#include "Framework/Events/CollisionEvent.h"

#include <btBulletCollisionCommon.h>

PlayerJumpResponder::PlayerJumpResponder(World& world, EventManager& eventManager)
	: world(world), eventManager(eventManager)
{
	ComponentBitmask bitmask;
	bitmask.setBit(world.getComponentId<PlayerComponent>(), true);
	bitmask.setBit(world.getComponentId<RigidbodyMotorComponent>(), true);
	this->eventManager.registerForEvent<CollisionEvent>(std::bind(&PlayerJumpResponder::handleCollisionEvent, this, std::placeholders::_1), bitmask);
}

void PlayerJumpResponder::handleCollisionEvent(const CollisionEvent& collisionEvent)
{
	if (collisionEvent.type != CollisionResponseType_Began) {
		return;
	}

	btCollisionObject* collidedBody = nullptr;
	if (*((eid_t*)collisionEvent.collisionManifold->getBody0()->getUserPointer()) == collisionEvent.target) {
		collidedBody = (btCollisionObject*)collisionEvent.collisionManifold->getBody0();
	} else {
		collidedBody = (btCollisionObject*)collisionEvent.collisionManifold->getBody1();
	}

	if ((collidedBody->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0) {
		// Don't collide with things like hurtboxes
		return;
	}

	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(collisionEvent.target);
	rigidbodyMotorComponent->canJump = true;
}
