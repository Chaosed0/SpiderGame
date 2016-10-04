
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
	requiredComponents1.setBit(world.getComponentId<PlayerComponent>(), true);
	requiredComponents2.setBit(world.getComponentId<HurtboxComponent>(), true);
	this->eventManager.registerForEvent<CollisionEvent>(std::bind(&HurtboxPlayerResponder::handleCollisionEvent, this, std::placeholders::_1));
}

void HurtboxPlayerResponder::handleCollisionEvent(const CollisionEvent& collisionEvent)
{
	eid_t e1 = collisionEvent.e1, e2 = collisionEvent.e2;
	if (!world.orderEntities(e1, e2, requiredComponents1, requiredComponents2)) {
		return;
	}

	if (collisionEvent.type != CollisionResponseType_Began) {
		return;
	}

	HurtboxComponent* hurtboxComponent = world.getComponent<HurtboxComponent>(e2);
	if (hurtboxComponent == nullptr) {
		return;
	}

	std::vector<eid_t>& ignoreEntities = hurtboxComponent->collidedEntities;
	if (std::find(ignoreEntities.begin(), ignoreEntities.end(), e1) != ignoreEntities.end()) {
		return;
	}

	DamageEvent damageEvent;
	damageEvent.target = e1;
	damageEvent.source = e2;
	damageEvent.damage = hurtboxComponent->data.damage;
	eventManager.sendEvent(damageEvent);

	ignoreEntities.push_back(e1);
}
