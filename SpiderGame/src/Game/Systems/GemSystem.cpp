
#include "GemSystem.h"

#include "Game/Components/GemComponent.h"
#include "Game/Components/PointlightComponent.h"

template <class T>
T lerp(T min, T max, float lerp)
{
	return min + (max - min) * lerp;
}

GemSystem::GemSystem(World& world, Renderer& renderer)
	: System(world),
	renderer(renderer)
{
	require<GemComponent>();
}

void GemSystem::updateEntity(float dt, eid_t entity)
{
	GemComponent* gemComponent = world.getComponent<GemComponent>(entity);

	PointLightComponent* pointLightComponent = world.getComponent<PointLightComponent>(gemComponent->light);
	assert (pointLightComponent != nullptr);

	PointLight light = renderer.getPointLight(pointLightComponent->handle);

	gemComponent->pulseTimer += dt;
	if (gemComponent->pulseTimer >= gemComponent->data.pulseTime) {
		gemComponent->pulseTimer -= gemComponent->data.pulseTime;
	}

	float interp = (sin(gemComponent->pulseTimer / gemComponent->data.pulseTime * glm::two_pi<float>()) + 1.0f) / 2.0f;
	light.constant = lerp(gemComponent->data.minIntensity.constant, gemComponent->data.maxIntensity.constant, interp);
	light.linear = lerp(gemComponent->data.minIntensity.linear, gemComponent->data.maxIntensity.linear, interp);
	light.quadratic = lerp(gemComponent->data.minIntensity.quadratic, gemComponent->data.maxIntensity.quadratic, interp);
	light.ambient = lerp(gemComponent->data.minIntensity.ambient, gemComponent->data.maxIntensity.ambient, interp);
	light.diffuse = lerp(gemComponent->data.minIntensity.diffuse, gemComponent->data.maxIntensity.diffuse, interp);
	light.specular = lerp(gemComponent->data.minIntensity.specular, gemComponent->data.maxIntensity.specular, interp);

	renderer.setPointLight(pointLightComponent->handle, light);
}
