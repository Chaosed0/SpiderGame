
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

	int lightIndex = pointLightComponent->data.pointLightIndex;
	if (lightIndex < 0) {
		return;
	}

	PointLight light = renderer.getPointLight(lightIndex);
	light.position = transformComponent->data->getWorldPosition();
	renderer.setPointLight(pointLightComponent->data.pointLightIndex, light);
}
