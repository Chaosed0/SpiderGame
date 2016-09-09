#pragma once

#include "Framework/System.h"

class SoundManager;

class AudioSourceSystem : public System
{
public:
	AudioSourceSystem(World& world, SoundManager& soundManager);
	void updateEntity(float dt, eid_t entity);
private:
	SoundManager& soundManager;
};