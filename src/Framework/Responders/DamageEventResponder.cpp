
#include "DamageEventResponder.h"

#include "Framework/Events/DamageEvent.h"
#include "Framework/Events/HealthChangedEvent.h"
#include "Framework/Components/HealthComponent.h"

DamageEventResponder::DamageEventResponder(World& world, EventManager& eventManager)
	: world(world), eventManager(eventManager)
{
	eventManager.registerForEvent<DamageEvent>(std::bind(&DamageEventResponder::damageReceived, this, std::placeholders::_1));
}

void DamageEventResponder::damageReceived(const DamageEvent& event)
{
	HealthComponent* healthComponent = world.getComponent<HealthComponent>(event.victim);
	healthComponent->health -= event.damage;

	HealthChangedEvent healthChangedEvent;
	healthChangedEvent.entity = event.victim;
	healthChangedEvent.healthChange = event.damage;
	healthChangedEvent.newHealth = healthComponent->health;
	eventManager.sendEvent(healthChangedEvent);
}
