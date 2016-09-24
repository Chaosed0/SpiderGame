#pragma once

#include "Framework/Component.h"
#include "Sound/AudioClip.h"

enum SpiderState
{
	SPIDER_IDLE,
	SPIDER_MOVING,
	SPIDER_ATTACKING,
	SPIDER_DEAD
};

class SpiderComponent : public Component
{
public:
	SpiderComponent() : animState(SPIDER_IDLE), attackTimer(0.0f), soundTimer(0.0f), soundTimeMin(3.0f), soundTimeMax(6.0f), soundTime(-1.0f), attackTime(1.0f) { }
	SpiderState animState;

	std::vector<AudioClip> sounds;
	AudioClip deathSound;

	float attackTime;
	float soundTimeMin;
	float soundTimeMax;

	/* Should only be set by system */
	float attackTimer;
	float soundTimer;
	float soundTime;
	bool madeHurtbox;
};