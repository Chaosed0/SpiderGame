#pragma once

#include "Framework/Component.h"

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
	SpiderComponent() : animState(SPIDER_IDLE), attackTimer(1.0f), attackTime(1.0f) { }
	SpiderState animState;
	float attackTimer;
	float attackTime;
};