
#include "VelocitySystem.h"

#include "Game/Components/VelocityComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/CollisionComponent.h"

#include "Util.h"

VelocitySystem::VelocitySystem(World& world)
	: System(world)
{
	require<TransformComponent>();
	require<VelocityComponent>();
}

void VelocitySystem::updateEntity(float dt, eid_t entity)
{
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);
	VelocityComponent* velocityComponent = world.getComponent<VelocityComponent>(entity);

	Transform& transform = transformComponent->transform;
	transform.setPosition(transform.getPosition() + transform.getRotation() * Util::forward * velocityComponent->speed * dt);
	transform.setRotation(transform.getRotation() * glm::angleAxis(velocityComponent->angularSpeed * dt, velocityComponent->rotationAxis));

	CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(entity);
	if (collisionComponent != nullptr && collisionComponent->controlsMovement == false) {
		collisionComponent->collisionObject->setWorldTransform(btTransform(Util::glmToBt(transform.getRotation()), Util::glmToBt(transform.getPosition())));
	}
}
