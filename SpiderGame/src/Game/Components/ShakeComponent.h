#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

#include <glm/glm.hpp>

class ShakeComponent : public Component
{
public:
	ShakeComponent() : timer(0.0f), active(true) { }

	struct Data {
		Data(float shakeTime, float frequency, float amplitude) : shakeTime(shakeTime), frequency(frequency), amplitude(amplitude) { }
		Data() : Data(3.0f, 1/30.0f, 0.15f) { }
		float shakeTime;
		float frequency;
		float amplitude;
	};

	Data data;
	float timer;
	std::vector<glm::vec3> samples;
	glm::vec3 currentOffset;
	bool active;
};

class ShakeConstructor : public DefaultComponentConstructor<ShakeComponent> {
	using DefaultComponentConstructor<ShakeComponent>::DefaultComponentConstructor;
};