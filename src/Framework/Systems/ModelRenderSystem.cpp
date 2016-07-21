
#include "ModelRenderSystem.h"
#include "Framework/Components/ModelRenderComponent.h"
#include "Framework/Components/TransformComponent.h"

ModelRenderSystem::ModelRenderSystem(World& world, Renderer& renderer)
	: System(world),
	renderer(renderer)
{
	require<ModelRenderComponent>();
	require<TransformComponent>();
	requireFinished();
}

void ModelRenderSystem::updateEntity(float dt, eid_t entity)
{
	ModelRenderComponent* modelComponent = world.getComponent<ModelRenderComponent>(entity);
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);

	renderer.setRenderableTransform(modelComponent->rendererHandle, transformComponent->transform);
}
