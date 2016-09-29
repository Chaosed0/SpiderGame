
#include "CollisionUpdateSystem.h"

#include "Game/Components/CollisionComponent.h"
#include "Game/Components/TransformComponent.h"

#include "Util.h"

CollisionUpdateSystem::CollisionUpdateSystem(World& world)
	: System(world)
{
	this->require<CollisionComponent>();
	this->require<TransformComponent>();
}

void CollisionUpdateSystem::updateEntity(float dt, eid_t entity)
{
	CollisionComponent* collisionComponent(world.getComponent<CollisionComponent>(entity));
	TransformComponent* transformComponent(world.getComponent<TransformComponent>(entity));

	std::shared_ptr<Transform>& transform = transformComponent->transform;
	if (collisionComponent->controlsMovement) {
		btTransform colTransform = collisionComponent->collisionObject->getWorldTransform();
		transform->setPosition(Util::btToGlm(colTransform.getOrigin()));
		transform->setRotation(Util::btToGlm(colTransform.getRotation()));
	} else {
		collisionComponent->collisionObject->setWorldTransform(btTransform(Util::glmToBt(transform->getWorldRotation()), Util::glmToBt(transform->getWorldPosition())));
	}
}