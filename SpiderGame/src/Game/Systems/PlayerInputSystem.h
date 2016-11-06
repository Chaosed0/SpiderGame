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
	void setDevice(Device device);
private:
	bool noclip;

	float horizontalRad;
	float verticalRad;
	Device device;

	EventManager& eventManager;
	Input& input;

	void tryActivate(eid_t player, PlayerComponent* playerComponent);
};