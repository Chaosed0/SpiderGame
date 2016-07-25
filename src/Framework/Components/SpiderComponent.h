#pragma once

#include "Framework/Component.h"

enum SpiderAnimState
{
	SPIDERANIM_IDLE,
	SPIDERANIM_MOVING,
	SPIDERANIM_DEAD
};

class SpiderComponent : public Component
{
public:
	SpiderComponent() : animState(SPIDERANIM_IDLE) { }
	SpiderAnimState animState;
};