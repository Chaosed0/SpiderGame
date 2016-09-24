
#include "Framework/ComponentBitmask.h"

class World;
class EventManager;
class DamageEvent;

class DamageEventResponder
{
public:
	DamageEventResponder(World& world, EventManager& eventManager);
private:
	void damageReceived(const DamageEvent& event);
	World& world;
	EventManager& eventManager;
	ComponentBitmask requiredComponents;
};