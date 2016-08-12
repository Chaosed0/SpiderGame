#pragma once

#include "Framework/ComponentBitmask.h"

class CollisionEvent;
class World;
class EventManager;

class PlayerJumpResponder
{
public:
	PlayerJumpResponder(World& world, EventManager& eventManager);
	void handleCollisionEvent(const CollisionEvent& collisionEvent);
private:
	World& world;
	EventManager& eventManager;
	ComponentBitmask requiredComponents;
};