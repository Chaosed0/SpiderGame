
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

	std::shared_ptr<Transform> transform = transformComponent->transform;
	transform->setPosition(transform->getPosition() + transform->getForward() * velocityComponent->speed * dt);
	transform->setRotation(transform->getRotation() * glm::angleAxis(velocityComponent->angularSpeed * dt, velocityComponent->rotationAxis));

}
