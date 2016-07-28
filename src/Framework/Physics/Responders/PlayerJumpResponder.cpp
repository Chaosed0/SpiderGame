
#include "PlayerJumpResponder.h"

#include "Framework/Components/RigidbodyMotorComponent.h"
#include "Framework/Components/PlayerComponent.h"

PlayerJumpResponder::PlayerJumpResponder(World& world)
	: CollisionResponder(world)
{
	this->requireOne<RigidbodyMotorComponent>();
	this->requireOne<PlayerComponent>();
}

void PlayerJumpResponder::collisionBegan(eid_t e1, eid_t e2, btPersistentManifold* contactManifold)
{
	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(e1);
	rigidbodyMotorComponent->canJump = true;
}

void PlayerJumpResponder::collisionEnded(eid_t e1, eid_t e2, btPersistentManifold* contactManifold)
{
}
