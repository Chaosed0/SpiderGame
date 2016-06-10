
#include "CameraSystem.h"

#include "Framework/Components/TransformComponent.h"
#include "Framework/Components/CameraComponent.h"

CameraSystem::CameraSystem(Renderer& renderer)
	: renderer(renderer)
{
	require<TransformComponent>();
	require<CameraComponent>();
}

void CameraSystem::updateEntity(float dt, Entity& entity)
{
	 CameraComponent* cameraComponent = entity.getComponent<CameraComponent>();
	 TransformComponent* transformComponent = entity.getComponent<TransformComponent>();

	 cameraComponent->camera.transform = transformComponent->transform;
}
