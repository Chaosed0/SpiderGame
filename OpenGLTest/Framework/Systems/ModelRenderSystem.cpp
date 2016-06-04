
#include "ModelRenderSystem.h"
#include "Framework/Components/ModelRenderComponent.h"
#include "Framework/Components/TransformComponent.h"

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
