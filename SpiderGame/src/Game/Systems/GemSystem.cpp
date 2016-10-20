
#include "GemSystem.h"

#include "Game/Components/TransformComponent.h"
#include "Game/Components/GemComponent.h"
#include "Game/Components/PointlightComponent.h"

#include "Game/Components/PlayerComponent.h"
#include "Game/Components/CollisionComponent.h"

#include "Game/Systems/GameEndingSystem.h"

#include <algorithm>

const float GemSystem::endGemHeight = 2.0f;
const float GemSystem::endGemAngularSpeed = 0.25f;
const float GemSystem::airLiftTime = 8.0f;

template <class T>
T lerp(T min, T max, float lerp)
{
	return min + (max - min) * lerp;
}

GemSystem::GemSystem(World& world, Renderer& renderer, EventManager& eventManager)
	: System(world),
	renderer(renderer),
	allGemsPlaced(false),
	eventManager(eventManager),
	endGameTime(GameEndingSystem::gemDefenseTime)
{
	require<GemComponent>();
	require<TransformComponent>();

	eventManager.registerForEvent<AllGemsCollectedEvent>(std::bind(&GemSystem::onAllGemsCollected, this, std::placeholders::_1));
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

	if (allGemsPlaced) {
		TransformComponent* gemTransformComponent = world.getComponent<TransformComponent>(entity);
		glm::vec3 currentGemPosition = gemTransformComponent->data->getWorldPosition();

		float heightChange = 0.0f;
		if (gemComponent->endGameTimer < this->airLiftTime) {
			heightChange = dt / this->airLiftTime * this->endGemHeight;
		}

		// Assume the gems circle around the origin
		float angularSpeed = (std::min)(gemComponent->endGameTimer / this->endGameTime, 1.0f) * this->endGemAngularSpeed;
		float theta = atan2f(currentGemPosition.x, currentGemPosition.z) + angularSpeed;
		float rad = glm::distance(currentGemPosition, glm::vec3(0.0f, currentGemPosition.y, 0.0f));

		glm::vec3 newPosition(sin(theta) * rad, currentGemPosition.y + heightChange, cos(theta) * rad);
		gemTransformComponent->data->setPosition(newPosition);

		gemComponent->endGameTimer += dt;
	}
}

void GemSystem::onAllGemsCollected(const AllGemsCollectedEvent& allGemsCollectedEvent)
{
	this->allGemsPlaced = true;

	std::vector<eid_t> gems = world.getEntitiesWithComponent<GemComponent>();
	for (unsigned i = 0; i < gems.size(); ++i) {
		world.removeComponent<CollisionComponent>(gems[i]);
	}
}
