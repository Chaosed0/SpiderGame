
#include "TestMovementSystem.h"

#include "TransformComponent.h"
#include "TestMovementComponent.h"

TestMovementSystem::TestMovementSystem()
{
	require<TransformComponent>();
	require<TestMovementComponent>();
}

void TestMovementSystem::updateEntity(float dt, Entity& entity)
{
	std::shared_ptr<TransformComponent> transformComponent = entity.getComponent<TransformComponent>();
	std::shared_ptr<TestMovementComponent> testMovementComponent = entity.getComponent<TestMovementComponent>();
	Transform& transform = transformComponent->transform;

	totalTime += dt;

	if (!testMovementComponent->initialized)
	{
		testMovementComponent->initialTransform = transform;
	}

	transform.setPosition(testMovementComponent->initialTransform.getPosition() + testMovementComponent->initialTransform.getRotation() * glm::vec3(0.0f, sin(totalTime / 5.0f) / 2.0f, 0.0f));
}
