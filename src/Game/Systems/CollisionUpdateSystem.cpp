
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

	btTransform transform = collisionComponent->collisionObject->getWorldTransform();
	transformComponent->transform.setPosition(Util::btToGlm(transform.getOrigin()));
	transformComponent->transform.setRotation(Util::btToGlm(transform.getRotation()));
}