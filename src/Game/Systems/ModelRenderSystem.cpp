
#include "ModelRenderSystem.h"
#include "Game/Components/ModelRenderComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Renderer/Renderer.h"

ModelRenderSystem::ModelRenderSystem(World& world, Renderer& renderer)
	: System(world),
	renderer(renderer)
{
	require<ModelRenderComponent>();
	require<TransformComponent>();
}

void ModelRenderSystem::updateEntity(float dt, eid_t entity)
{
	ModelRenderComponent* modelComponent = world.getComponent<ModelRenderComponent>(entity);
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);

	renderer.setRenderableTransform(modelComponent->rendererHandle, transformComponent->transform);
}
