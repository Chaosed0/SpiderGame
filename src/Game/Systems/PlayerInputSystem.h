#pragma once

#include <glm/glm.hpp>

#include "Framework/System.h"
#include "Framework/EventManager.h"
#include "Input/Input.h"

class PlayerComponent;

class PlayerInputSystem : public System
{
public:
	PlayerInputSystem(World& world, Input& input, EventManager& eventManager);
	void updateEntity(float dt, eid_t entity);

	void setNoclip(bool noclip);

	float getCameraVertical();
private:
	bool noclip;

	float horizontalRad;
	float verticalRad;

	EventManager& eventManager;
	Input& input;

	void tryActivate(PlayerComponent* playerComponent);
};