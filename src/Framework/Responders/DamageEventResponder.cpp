
#include "DamageEventResponder.h"

#include "Framework/Events/DamageEvent.h"
#include "Framework/Events/HealthChangedEvent.h"
#include "Framework/Components/HealthComponent.h"

DamageEventResponder::DamageEventResponder(World& world, EventManager& eventManager)
	: world(world), eventManager(eventManager)
{
	ComponentBitmask bitmask;
	bitmask.setBit(world.getComponentId<HealthComponent>(), true);

	using namespace std::placeholders;
	eventManager.registerForEvent<DamageEvent>(std::bind(&DamageEventResponder::damageReceived, this, _1, _2), bitmask);
}

void DamageEventResponder::damageReceived(const DamageEvent& event, eid_t entity)
{
	HealthComponent* healthComponent = world.getComponent<HealthComponent>(entity);
	healthComponent->health -= event.damage;

	HealthChangedEvent healthChangedEvent;
	healthChangedEvent.healthChange = event.damage;
	healthChangedEvent.newHealth = healthComponent->health;
	eventManager.sendEvent(healthChangedEvent, entity);
}
