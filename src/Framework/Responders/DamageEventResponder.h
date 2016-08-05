
#include "Framework/World.h"
#include "Framework/EventManager.h"

class DamageEvent;

class DamageEventResponder
{
public:
	DamageEventResponder(World& world, EventManager& eventManager);
private:
	void damageReceived(const DamageEvent& event);
	World& world;
	EventManager& eventManager;
};