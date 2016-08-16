#pragma once

#include "Framework/Component.h"

#include <glm/glm.hpp>

struct VelocityComponent : public Component
{
	float speed;
	float angularSpeed;
	glm::vec3 rotationAxis;
};