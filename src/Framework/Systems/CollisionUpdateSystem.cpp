
#include "CollisionUpdateSystem.h"

#include "Framework/Components/CollisionComponent.h"
#include "Framework/Components/TransformComponent.h"

#include "Util.h"

CollisionUpdateSystem::CollisionUpdateSystem(World& world)
	: System(world)
{
	this->require<CollisionComponent>();
	this->require<TransformComponent>();
	this->requireFinished();
}

void CollisionUpdateSystem::updateEntity(float dt, eid_t entity)
{
	CollisionComponent* collisionComponent(world.getComponent<CollisionComponent>(entity));
	TransformComponent* transformComponent(world.getComponent<TransformComponent>(entity));

	btTransform transform = collisionComponent->body->getWorldTransform();
	transformComponent->transform.setPosition(Util::btToGlm(transform.getOrigin()));
	transformComponent->transform.setRotation(Util::btToGlm(transform.getRotation()));
}