
#include "CollisionUpdateSystem.h"

#include "Framework/Components/CollisionComponent.h"
#include "Framework/Components/TransformComponent.h"

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
	transformComponent->transform.setPosition(glm::vec3(transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z()));
	transformComponent->transform.setRotation(glm::quat(transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z(), transform.getRotation().w()));
}