
#include "Framework/World.h"
#include "Framework/EventManager.h"

class DamageEvent;

class DamageEventResponder
{
public:
	DamageEventResponder(World& world, EventManager& eventManager);
private:
	void damageReceived(const DamageEvent& event, eid_t entity);
	World& world;
	EventManager& eventManager;
};