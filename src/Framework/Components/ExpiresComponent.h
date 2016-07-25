#pragma once

#include "Framework/Component.h"

class ExpiresComponent : public Component
{
public:
	ExpiresComponent() : timer(0.0f), expiryTime(0.0f) { }

	float timer;
	float expiryTime;
};