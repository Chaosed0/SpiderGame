#pragma once

#include <glm/glm.hpp>

#include "Framework/System.h"

class PlayerInputSystem : public System
{
public:
	PlayerInputSystem(World& world);
	void updateEntity(float dt, eid_t entity);

	void startMoving(glm::vec2 movement);
	void stopMoving(glm::vec2 movement);
	void startJump();
	void setNoclip(bool noclip);

	void rotateCamera(float horizontal, float vertical);
	float getCameraVertical();
private:
	bool forward, back, left, right;
	bool jump;
	bool noclip;

	float rotateHorizontal;
	float rotateVertical;

	float horizontalRad;
	float verticalRad;
};