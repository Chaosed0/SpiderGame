#pragma once

#include <glm/glm.hpp>

#include "Framework/System.h"
#include "Framework/EventManager.h"

class PlayerComponent;

class PlayerInputSystem : public System
{
public:
	PlayerInputSystem(World& world, EventManager& eventManager);
	void updateEntity(float dt, eid_t entity);

	void startMoving(glm::vec2 movement);
	void stopMoving(glm::vec2 movement);
	void setShooting(bool shooting);
	void startJump();
	void setNoclip(bool noclip);
	void activate();

	void rotateCamera(float horizontal, float vertical);
	float getCameraVertical();
private:
	bool forward, back, left, right;
	bool jump;
	bool noclip;
	bool shooting;
	bool activating;

	float rotateHorizontal;
	float rotateVertical;

	float horizontalRad;
	float verticalRad;

	EventManager& eventManager;

	void tryActivate(PlayerComponent* playerComponent);
};