#pragma once

#include "Framework/Component.h"

struct ShootingComponent : public Component
{
	ShootingComponent() : shotRequested(false), maxShotDistance(100.0f) {}
	bool shotRequested;
	float maxShotDistance;
};
