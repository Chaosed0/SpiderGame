
#include "CameraSystem.h"

#include "Game/Components/TransformComponent.h"
#include "Game/Components/CameraComponent.h"

CameraSystem::CameraSystem(World& world, Renderer& renderer)
	: System(world), renderer(renderer)
{
	require<TransformComponent>();
	require<CameraComponent>();
}

void CameraSystem::updateEntity(float dt, eid_t entity)
{
	 CameraComponent* cameraComponent = world.getComponent<CameraComponent>(entity);
	 TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);

	 cameraComponent->data.setInverseViewMatrix(transformComponent->data->matrix());
	 if (cameraComponent->isActive) {
		 renderer.setProjectionMatrix(cameraComponent->data.getProjectionMatrix());
		 renderer.setViewMatrix(cameraComponent->data.getViewMatrix());
	 }
}
