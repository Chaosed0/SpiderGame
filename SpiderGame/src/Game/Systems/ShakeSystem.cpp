
#include "ShakeSystem.h"

#include "Game/Components/ShakeComponent.h"
#include "Game/Components/TransformComponent.h"

#include "Util.h"

ShakeSystem::ShakeSystem(World& world, std::default_random_engine& generator)
	: System(world),
	generator(generator)
{
	require<TransformComponent>();
	require<ShakeComponent>();
}

void ShakeSystem::updateEntity(float dt, eid_t entity)
{
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);
	ShakeComponent* shakeComponent = world.getComponent<ShakeComponent>(entity);

	shakeComponent->timer += dt;

	float timer = shakeComponent->timer;
	float frequency = shakeComponent->data.frequency;
	float time = shakeComponent->data.shakeTime;

	if (!shakeComponent->active || (time > 0.0f && timer >= time)) {
		shakeComponent->currentOffset = glm::vec3(0.0f);
		return;
	}

	int sampleIndex = (int)std::floor(timer / frequency);
	float interval = timer - sampleIndex * frequency;

	printf("%d %g\n", sampleIndex, timer);

	// Generate a new sample
	if (sampleIndex > shakeComponent->sampleIndex) {
		++shakeComponent->sampleIndex;
		shakeComponent->currentSample = shakeComponent->nextSample;

		// Check if we are about to end the shake
		if (time > 0.0f && time - timer < frequency) {
			shakeComponent->nextSample = glm::vec3(0.0f);
		} else {
			std::uniform_real_distribution<float> sampleRand(-shakeComponent->data.amplitude, shakeComponent->data.amplitude);
			shakeComponent->nextSample = glm::vec3(sampleRand(generator), sampleRand(generator), 0.0f);
		}
	}

	float lerp = interval / shakeComponent->data.frequency;
	shakeComponent->currentOffset = Util::interpolate(shakeComponent->currentSample, shakeComponent->nextSample, lerp);
}
