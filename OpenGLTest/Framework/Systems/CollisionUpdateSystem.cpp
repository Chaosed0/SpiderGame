
#include "CollisionUpdateSystem.h"

#include "Framework/Components/CollisionComponent.h"
#include "Framework/Components/TransformComponent.h"

#include "Util.h"

CollisionUpdateSystem::CollisionUpdateSystem()
{
	this->require<CollisionComponent>();
	this->require<TransformComponent>();
}

void CollisionUpdateSystem::updateEntity(float dt, Entity& entity)
{
	CollisionComponent* collisionComponent(entity.getComponent<CollisionComponent>());
	TransformComponent* transformComponent(entity.getComponent<TransformComponent>());

	btTransform transform = collisionComponent->body->getWorldTransform();
	transformComponent->transform.setPosition(Util::btToGlm(transform.getOrigin()));
	transformComponent->transform.setRotation(Util::btToGlm(transform.getRotation()));
}