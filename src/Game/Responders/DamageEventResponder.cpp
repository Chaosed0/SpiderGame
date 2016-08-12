
#include "DamageEventResponder.h"

#include "Framework/World.h"
#include "Game/Components/HealthComponent.h"

#include "Framework/EventManager.h"
#include "Game/Events/DamageEvent.h"
#include "Game/Events/HealthChangedEvent.h"

DamageEventResponder::DamageEventResponder(World& world, EventManager& eventManager)
	: world(world), eventManager(eventManager)
{
	requiredComponents.setBit(world.getComponentId<HealthComponent>(), true);

	eventManager.registerForEvent<DamageEvent>(std::bind(&DamageEventResponder::damageReceived, this, std::placeholders::_1));
}

void DamageEventResponder::damageReceived(const DamageEvent& event)
{
	if (!world.entityHasComponents(event.target, requiredComponents)) {
		return;
	}

	HealthComponent* healthComponent = world.getComponent<HealthComponent>(event.target);
	healthComponent->health -= event.damage;

	HealthChangedEvent healthChangedEvent;
	healthChangedEvent.entity = event.target;
	healthChangedEvent.healthChange = event.damage;
	healthChangedEvent.newHealth = healthComponent->health;
	eventManager.sendEvent(healthChangedEvent);
}
