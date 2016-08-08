
#include "HurtboxPlayerResponder.h"

#include "Game/Components/HealthComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Game/Components/HurtboxComponent.h"

#include "Framework/EventManager.h"
#include "Framework/CollisionEvent.h"
#include "Game/Events/DamageEvent.h"

#include <btBulletCollisionCommon.h>

HurtboxPlayerResponder::HurtboxPlayerResponder(World& world, EventManager& eventManager)
	: world(world),
	eventManager(eventManager)
{
	ComponentBitmask bitmask;
	bitmask.setBit(world.getComponentId<PlayerComponent>(), true);
	this->eventManager.registerForEvent<CollisionEvent>(std::bind(&HurtboxPlayerResponder::handleCollisionEvent, this, std::placeholders::_1), bitmask);
}

void HurtboxPlayerResponder::handleCollisionEvent(const CollisionEvent& collisionEvent)
{
	if (collisionEvent.type != CollisionResponseType_Began) {
		return;
	}

	HurtboxComponent* hurtboxComponent = world.getComponent<HurtboxComponent>(collisionEvent.collidedEntity);
	if (hurtboxComponent == nullptr) {
		return;
	}

std::vector<eid_t>& ignoreEntities = hurtboxComponent->collidedEntities;
	if (std::find(ignoreEntities.begin(), ignoreEntities.end(), collisionEvent.target) != ignoreEntities.end()) {
		return;
	}

	DamageEvent damageEvent;
	damageEvent.target = collisionEvent.target;
	damageEvent.source = collisionEvent.collidedEntity;
	damageEvent.damage = hurtboxComponent->damage;
	eventManager.sendEvent(damageEvent);

	ignoreEntities.push_back(collisionEvent.target);
}
