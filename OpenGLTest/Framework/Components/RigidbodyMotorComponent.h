#pragma once

#include <glm/glm.hpp>

#include "Framework/Component.h"

class RigidbodyMotorComponent : public Component
{
public:
	float moveSpeed;
	float jumpSpeed;
	glm::vec2 movement;
	bool jump;
	bool canJump;
};