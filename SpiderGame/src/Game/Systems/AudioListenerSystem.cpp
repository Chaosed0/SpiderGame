
#include "AudioListenerSystem.h"
#include "Game/Components/AudioListenerComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Sound/SoundManager.h"

AudioListenerSystem::AudioListenerSystem(World& world, SoundManager& soundManager)
	: System(world),
	soundManager(soundManager)
{
	require<AudioListenerComponent>();
	require<TransformComponent>();
}

void AudioListenerSystem::updateEntity(float dt, eid_t entity)
{
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);

	soundManager.setListenerTransform(transformComponent->transform->getWorldPosition(), transformComponent->transform->getWorldRotation());
}
