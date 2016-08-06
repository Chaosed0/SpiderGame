
#include "DamageEventResponder.h"

#include "Framework/World.h"
#include "Framework/Components/HealthComponent.h"

#include "Framework/EventManager.h"
#include "Framework/Events/DamageEvent.h"
#include "Framework/Events/HealthChangedEvent.h"

DamageEventResponder::DamageEventResponder(World& world, EventManager& eventManager)
	: world(world), eventManager(eventManager)
{
	ComponentBitmask bitmask;
	bitmask.setBit(world.getComponentId<HealthComponent>(), true);

	eventManager.registerForEvent<DamageEvent>(std::bind(&DamageEventResponder::damageReceived, this, std::placeholders::_1), bitmask);
}

void DamageEventResponder::damageReceived(const DamageEvent& event)
{
	HealthComponent* healthComponent = world.getComponent<HealthComponent>(event.target);
	healthComponent->health -= event.damage;

	HealthChangedEvent healthChangedEvent;
	healthChangedEvent.target = event.target;
	healthChangedEvent.healthChange = event.damage;
	healthChangedEvent.newHealth = healthComponent->health;
	eventManager.sendEvent(healthChangedEvent);
}
