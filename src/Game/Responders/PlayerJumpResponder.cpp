
#include "PlayerJumpResponder.h"

#include "Framework/Physics.h"
#include "Framework/World.h"
#include "Framework/EventManager.h"

#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Framework/CollisionEvent.h"

#include <btBulletCollisionCommon.h>

PlayerJumpResponder::PlayerJumpResponder(World& world, EventManager& eventManager)
	: world(world), eventManager(eventManager)
{
	requiredComponents.setBit(world.getComponentId<PlayerComponent>(), true);
	requiredComponents.setBit(world.getComponentId<RigidbodyMotorComponent>(), true);
	this->eventManager.registerForEvent<CollisionEvent>(std::bind(&PlayerJumpResponder::handleCollisionEvent, this, std::placeholders::_1));
}

void PlayerJumpResponder::handleCollisionEvent(const CollisionEvent& collisionEvent)
{
	eid_t e1 = collisionEvent.e1, e2 = collisionEvent.e2;
	if (!world.orderEntities(e1, e2, requiredComponents, ComponentBitmask())) {
		return;
	}

	if (collisionEvent.type != CollisionResponseType_Began) {
		return;
	}

	btCollisionObject* collidedBody = nullptr;
	if (*((eid_t*)collisionEvent.collisionManifold->getBody0()->getUserPointer()) == e1) {
		collidedBody = (btCollisionObject*)collisionEvent.collisionManifold->getBody1();
	} else {
		collidedBody = (btCollisionObject*)collisionEvent.collisionManifold->getBody0();
	}

	if ((collidedBody->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0) {
		// Don't collide with things like hurtboxes
		return;
	}

	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(e1);
	rigidbodyMotorComponent->canJump = true;
}
