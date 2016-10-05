#pragma once

#include "Framework/Component.h"
#include "Framework/ComponentConstructor.h"
#include "Sound/SoundManager.h"

struct AudioSourceComponent : public Component
{
	AudioSourceComponent() : sourceHandle(nullptr) { }
	SoundManager::SourceHandle sourceHandle;
};

class AudioSourceConstructor : public ComponentConstructor
{
public:
	AudioSourceConstructor(SoundManager& soundManager) : soundManager(soundManager) { }
	virtual ComponentConstructorInfo construct(World& world, eid_t parent, void* userinfo) const
	{
		AudioSourceComponent* component = new AudioSourceComponent();
		component->sourceHandle = soundManager.getSourceHandle();
		return ComponentConstructorInfo(component, typeid(AudioSourceComponent).hash_code());
	}
private:
	SoundManager& soundManager;
};