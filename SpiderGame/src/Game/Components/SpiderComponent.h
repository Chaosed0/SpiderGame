#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Sound/AudioClip.h"

enum SpiderState
{
	SPIDER_IDLE,
	SPIDER_MOVING,
	SPIDER_PREPARING_LEAP,
	SPIDER_LEAPING,
	SPIDER_LEAP_RECOVERY,
	SPIDER_DEAD
};

class SpiderComponent : public Component
{
public:
	SpiderComponent() : animState(SPIDER_IDLE), timer(0.0f), soundTimer(0.0f), soundTime(-1.0f) { }

	struct Data {
		Data() : soundTimeMin(3.0f), soundTimeMax(6.0f), attackTime(0.5f), recoveryTime(0.25f), normalMoveSpeed(3.5f), leapMoveSpeed(7.0f) { }
		std::vector<AudioClip> sounds;
		AudioClip deathSound;

		float attackTime;
		float recoveryTime;
		float soundTimeMin;
		float soundTimeMax;
		float normalMoveSpeed;
		float leapMoveSpeed;
	};

	Data data;

	float timer;
	float soundTimer;
	float soundTime;
	eid_t hurtbox;
	SpiderState animState;
};

class SpiderConstructor : public DefaultComponentConstructor<SpiderComponent> {
public:
	using DefaultComponentConstructor<SpiderComponent>::DefaultComponentConstructor;
};