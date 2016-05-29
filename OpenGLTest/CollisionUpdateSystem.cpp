
#include "CollisionUpdateSystem.h"

#include "CollisionComponent.h"
#include "TransformComponent.h"

CollisionUpdateSystem::CollisionUpdateSystem()
{
	this->require<CollisionComponent>();
	this->require<TransformComponent>();
}

void CollisionUpdateSystem::updateEntity(float dt, Entity& entity)
{
	std::shared_ptr<CollisionComponent> collisionComponent(entity.getComponent<CollisionComponent>());
	std::shared_ptr<TransformComponent> transformComponent(entity.getComponent<TransformComponent>());

	btTransform transform = collisionComponent->body->getWorldTransform();
	transformComponent->transform.setPosition(glm::vec3(transform.getOrigin().x(), transform.getOrigin().y(), transform.getOrigin().z()));
	transformComponent->transform.setRotation(glm::quat(transform.getRotation().x(), transform.getRotation().y(), transform.getRotation().z(), transform.getRotation().w()));
}