#pragma once

#include "Framework/Component.h"
#include "Framework/Components/TransformComponent.h"

class FollowComponent : public Component
{
public:
	FollowComponent() : target(nullptr), repathInterval(1.0f), repathTimer(0.0f) { }
	TransformComponent* target;
	float repathInterval;
	float repathTimer;
};