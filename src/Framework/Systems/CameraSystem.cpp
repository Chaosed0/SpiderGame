
#include "CameraSystem.h"

#include "Framework/Components/TransformComponent.h"
#include "Framework/Components/CameraComponent.h"

CameraSystem::CameraSystem(World& world, Renderer& renderer)
	: System(world),
	renderer(renderer)
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
