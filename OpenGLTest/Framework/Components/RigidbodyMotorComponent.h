#pragma once

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>

#include "Framework/Component.h"

class RigidbodyMotorComponent : public Component
{
public:
	float moveSpeed;
	float jumpSpeed;
	glm::quat facing;
	glm::vec2 movement;
	bool jump;
	bool noclip;
	bool canJump;
};