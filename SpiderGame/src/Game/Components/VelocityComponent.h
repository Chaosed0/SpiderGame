#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

#include <glm/glm.hpp>

struct VelocityComponent : public Component
{
	struct Data {
		Data(float speed, float angularSpeed, glm::vec3 rotationAxis)
			: speed(speed), angularSpeed(angularSpeed), rotationAxis(rotationAxis) { }
		Data(float angularSpeed, glm::vec3 rotationAxis) : Data(0.0f, angularSpeed, rotationAxis) { }
		Data(float speed) : Data(speed, 0.0f, glm::vec3(0.0f)) { }
		Data() : Data(0.0f, 0.0f, glm::vec3(0.0f)) { }
		float speed;
		float angularSpeed;
		glm::vec3 rotationAxis;
	};

	Data data;
};

class VelocityConstructor : public DefaultComponentConstructor<VelocityComponent> {
public:
	using DefaultComponentConstructor<VelocityComponent>::DefaultComponentConstructor;
};