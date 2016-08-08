#pragma once

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
};