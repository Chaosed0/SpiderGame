
#include "PointLightSystem.h"
#include "Game/Components/PointLightComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Renderer/Renderer.h"

PointLightSystem::PointLightSystem(World& world, Renderer& renderer)
	: System(world),
	renderer(renderer)
{
	require<PointLightComponent>();
	require<TransformComponent>();
}

void PointLightSystem::updateEntity(float dt, eid_t entity)
{
	PointLightComponent* pointLightComponent = world.getComponent<PointLightComponent>(entity);
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);

	PointLight light = renderer.getPointLight(pointLightComponent->handle);
	light.position = transformComponent->data->getWorldPosition();
	renderer.setPointLight(pointLightComponent->handle, light);
}
