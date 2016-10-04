#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>

class RigidbodyMotorComponent : public Component
{
public:
	RigidbodyMotorComponent()
		: jump(false), noclip(false), canJump(false), canMove(true) { }

	struct Data {
		Data(float moveSpeed, float jumpSpeed) : moveSpeed(moveSpeed), jumpSpeed(jumpSpeed) { }
		Data() : Data(1.0f, 1.0f) { }
		float moveSpeed;
		float jumpSpeed;
	};

	Data data;

	glm::quat facing;
	glm::vec2 movement;
	bool jump;
	bool noclip;
	bool canJump;
	bool canMove;
};

class RigidbodyMotorConstructor : public DefaultComponentConstructor<RigidbodyMotorComponent> {
public:
	using DefaultComponentConstructor<RigidbodyMotorComponent>::DefaultComponentConstructor;
};