
#include "ModelRenderSystem.h"
#include "ModelRenderComponent.h"
#include "TransformComponent.h"

ModelRenderSystem::ModelRenderSystem(Renderer& renderer)
	: renderer(renderer)
{
	require<ModelRenderComponent>();
	require<TransformComponent>();
}

void ModelRenderSystem::updateEntity(float dt, Entity& entity)
{
	std::shared_ptr<ModelRenderComponent> modelComponent = entity.getComponent<ModelRenderComponent>();
	std::shared_ptr<TransformComponent> transformComponent = entity.getComponent<TransformComponent>();

	renderer.updateTransform(modelComponent->rendererHandle, transformComponent->transform);
}
