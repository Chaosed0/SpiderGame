#pragma once

#include "Framework/ComponentBitmask.h"

class CollisionEvent;
class World;
class EventManager;

class PlayerJumpResponder
{
public:
	PlayerJumpResponder(World& world, EventManager& eventManager);
private:
	World& world;
	EventManager& eventManager;
	ComponentBitmask requiredComponents;

	void handleCollisionEvent(const CollisionEvent& collisionEvent);
};