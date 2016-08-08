
#include "CameraSystem.h"

#include "Game/Components/TransformComponent.h"
#include "Game/Components/CameraComponent.h"

CameraSystem::CameraSystem(World& world)
	: System(world)
{
	require<TransformComponent>();
	require<CameraComponent>();
}

void CameraSystem::updateEntity(float dt, eid_t entity)
{
	 CameraComponent* cameraComponent = world.getComponent<CameraComponent>(entity);
	 TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);

	 cameraComponent->camera.inverseViewMatrix = transformComponent->transform.matrix();
}
