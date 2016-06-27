
#include "TestMovementSystem.h"

#include "Framework/Components/TransformComponent.h"
#include "Framework/Components/TestMovementComponent.h"

TestMovementSystem::TestMovementSystem()
{
	require<TransformComponent>();
	require<TestMovementComponent>();
}

void TestMovementSystem::updateEntity(float dt, Entity& entity)
{
	TransformComponent* transformComponent = entity.getComponent<TransformComponent>();
	TestMovementComponent* testMovementComponent = entity.getComponent<TestMovementComponent>();
	Transform& transform = transformComponent->transform;

	totalTime += dt;

	if (!testMovementComponent->initialized)
	{
		testMovementComponent->initialTransform = transform;
	}

	transform.setPosition(testMovementComponent->initialTransform.getPosition() + testMovementComponent->initialTransform.getRotation() * glm::vec3(0.0f, sin(totalTime / 5.0f) / 2.0f, 0.0f));
}
