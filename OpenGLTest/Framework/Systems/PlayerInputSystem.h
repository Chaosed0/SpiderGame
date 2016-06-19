#pragma once

#include <glm/glm.hpp>

#include "Framework/System.h"

class PlayerInputSystem : public System
{
public:
	PlayerInputSystem();
	void updateEntity(float dt, Entity& entity);

	void startMoving(glm::vec2 movement);
	void stopMoving(glm::vec2 movement);
	void startJump();
	void setHorizontalVerticalRotation(float horizontal, float vertical);
private:
	bool forward, back, left, right;
	bool jump;

	float horizontal;
	float vertical;
};