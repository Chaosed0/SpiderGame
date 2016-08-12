#pragma once

#include "Framework/ComponentBitmask.h"

class World;
class EventManager;
class CollisionEvent;

class HurtboxPlayerResponder
{
public:
	HurtboxPlayerResponder(World& world, EventManager& eventManager);
	void handleCollisionEvent(const CollisionEvent& event);
private:
	World& world;
	EventManager& eventManager;
	ComponentBitmask requiredComponents1;
	ComponentBitmask requiredComponents2;
};