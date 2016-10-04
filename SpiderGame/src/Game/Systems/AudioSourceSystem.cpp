
#include "AudioSourceSystem.h"
#include "Game/Components/AudioSourceComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Sound/SoundManager.h"

AudioSourceSystem::AudioSourceSystem(World& world, SoundManager& soundManager)
	: System(world),
	soundManager(soundManager)
{
	require<AudioSourceComponent>();
	require<TransformComponent>();
}

void AudioSourceSystem::updateEntity(float dt, eid_t entity)
{
	AudioSourceComponent* audioSourceComponent = world.getComponent<AudioSourceComponent>(entity);
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);

	soundManager.setSourcePosition(audioSourceComponent->sourceHandle, transformComponent->data->getWorldPosition());
}
