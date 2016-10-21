
#include "CameraSystem.h"

#include "Game/Components/TransformComponent.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Components/ShakeComponent.h"

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
	 ShakeComponent* shakeComponent = world.getComponent<ShakeComponent>(entity);

	 glm::mat4 matrix = transformComponent->data->matrix();
	 if (shakeComponent != nullptr) {
		 matrix = glm::translate(matrix, shakeComponent->currentOffset);
	 }
	 cameraComponent->data.setInverseViewMatrix(matrix);

	 if (cameraComponent->isActive) {
		 renderer.setProjectionMatrix(cameraComponent->data.getProjectionMatrix());
		 renderer.setViewMatrix(cameraComponent->data.getViewMatrix());
	 }
}
