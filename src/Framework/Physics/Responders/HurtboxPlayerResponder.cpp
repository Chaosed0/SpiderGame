
#include "HurtboxPlayerResponder.h"

#include "Framework/Components/HealthComponent.h"
#include "Framework/Components/PlayerComponent.h"
#include "Framework/Components/HurtboxComponent.h"

#include "Framework/Events/DamageEvent.h"

HurtboxPlayerResponder::HurtboxPlayerResponder(World& world, EventManager& eventManager)
	: CollisionResponder(world),
	eventManager(eventManager)
{
	this->requireOne<HurtboxComponent>();
	this->requireTwo<PlayerComponent>();
	this->requireTwo<HealthComponent>();
}

void HurtboxPlayerResponder::collisionBegan(eid_t e1, eid_t e2, btPersistentManifold* contactManifold)
{
	HurtboxComponent* hurtboxComponent = world.getComponent<HurtboxComponent>(e1);

	std::vector<eid_t>& ignoreEntities = hurtboxComponent->collidedEntities;
	if (std::find(ignoreEntities.begin(), ignoreEntities.end(), e2) != ignoreEntities.end()) {
		return;
	}

	DamageEvent damageEvent;
	damageEvent.source = e1;
	damageEvent.damage = hurtboxComponent->damage;
	eventManager.sendEvent(damageEvent, e2);

	ignoreEntities.push_back(e2);
}

void HurtboxPlayerResponder::collisionEnded(eid_t e1, eid_t e2, btPersistentManifold* contactManifold)
{
}
