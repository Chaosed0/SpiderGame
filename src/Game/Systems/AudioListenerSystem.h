#pragma once

#include "Framework/System.h"

class SoundManager;

class AudioListenerSystem : public System
{
public:
	AudioListenerSystem(World& world, SoundManager& soundManager);
	void updateEntity(float dt, eid_t entity);
private:
	SoundManager& soundManager;
};