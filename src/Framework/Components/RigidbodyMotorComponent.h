#pragma once

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>

#include "Framework/Component.h"

class RigidbodyMotorComponent : public Component
{
public:
	RigidbodyMotorComponent() : moveSpeed(1.0f), jumpSpeed(1.0f), jump(false), noclip(false), canJump(false) { }
	float moveSpeed;
	float jumpSpeed;
	glm::quat facing;
	glm::vec2 movement;
	bool jump;
	bool noclip;
	bool canJump;
};