
#include "PlayerJumpResponder.h"

#include "Framework/Physics.h"
#include "Framework/World.h"
#include "Framework/EventManager.h"

#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Framework/CollisionEvent.h"

PlayerJumpResponder::PlayerJumpResponder(World& world, EventManager& eventManager)
	: world(world), eventManager(eventManager)
{
	requiredComponents.setBit(world.getComponentId<PlayerComponent>(), true);
	requiredComponents.setBit(world.getComponentId<RigidbodyMotorComponent>(), true);
	this->eventManager.registerForEvent<CollisionEvent>(std::bind(&PlayerJumpResponder::handleCollisionEvent, this, std::placeholders::_1));
}

void PlayerJumpResponder::handleCollisionEvent(const CollisionEvent& collisionEvent)
{
	eid_t player = collisionEvent.e1;
	eid_t other = collisionEvent.e2;
	if (!world.orderEntities(player, other, requiredComponents, ComponentBitmask())) {
		return;
	}

	btCollisionObject* playerBody = nullptr;
	btCollisionObject* otherBody = nullptr;
	if (*((eid_t*)collisionEvent.collisionManifold->getBody0()->getUserPointer()) == player) {
		playerBody = (btCollisionObject*)collisionEvent.collisionManifold->getBody0();
		otherBody = (btCollisionObject*)collisionEvent.collisionManifold->getBody1();
	} else {
		otherBody = (btCollisionObject*)collisionEvent.collisionManifold->getBody0();
		playerBody = (btCollisionObject*)collisionEvent.collisionManifold->getBody1();
	}

	if ((otherBody->getCollisionFlags() & btCollisionObject::CF_NO_CONTACT_RESPONSE) != 0) {
		// Don't collide with things like hurtboxes
		return;
	}

	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(player);
	if (collisionEvent.type == CollisionResponseType_Began) {
		++rigidbodyMotorComponent->numContacts;
	} else {
		--rigidbodyMotorComponent->numContacts;
	}

	printf("%d\n", rigidbodyMotorComponent->numContacts);

	rigidbodyMotorComponent->canJump = (rigidbodyMotorComponent->numContacts >= 0);
}
