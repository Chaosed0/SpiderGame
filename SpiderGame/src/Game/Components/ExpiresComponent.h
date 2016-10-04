#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

class ExpiresComponent : public Component
{
public:
	ExpiresComponent() : timer(0.0f) { }

	struct Data {
		Data(float expiryTime) : expiryTime(expiryTime) { }
		Data() : Data(0.0f) { }
		float expiryTime;
	};

	Data data;
	float timer;
};

class ExpiresConstructor : public DefaultComponentConstructor<ExpiresComponent> {
	using DefaultComponentConstructor<ExpiresComponent>::DefaultComponentConstructor;
};