
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

	if (!shakeComponent->active || shakeComponent->timer >= shakeComponent->data.shakeTime) {
		shakeComponent->currentOffset = glm::vec3(0.0f);
		return;
	}

	if (shakeComponent->timer == 0.0f) {
		std::vector<glm::vec3> samples;
		samples.resize((int)std::ceil(shakeComponent->data.shakeTime / shakeComponent->data.frequency));
		std::uniform_real_distribution<float> sampleRand(-shakeComponent->data.amplitude, shakeComponent->data.amplitude);
		for (unsigned i = 0; i < samples.size(); ++i) {
			samples[i] = glm::vec3(sampleRand(generator), sampleRand(generator), 0.0f);
		}

		samples.push_back(glm::vec3(0.0f));
		shakeComponent->samples = samples;
	}

	int sampleIndex = (int)std::floor(shakeComponent->timer / shakeComponent->data.frequency);
	glm::vec3 s1 = shakeComponent->samples[sampleIndex];
	glm::vec3 s2 = shakeComponent->samples[sampleIndex+1];
	float lerp = shakeComponent->timer / shakeComponent->data.frequency - sampleIndex;

	shakeComponent->currentOffset = Util::interpolate(s1, s2, lerp);
	shakeComponent->timer += dt;
}
