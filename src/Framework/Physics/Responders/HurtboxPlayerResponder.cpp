
#include "HurtboxPlayerResponder.h"

#include "Framework/Components/HealthComponent.h"
#include "Framework/Components/PlayerComponent.h"
#include "Framework/Components/HurtboxComponent.h"

HurtboxPlayerResponder::HurtboxPlayerResponder(World& world)
	: CollisionResponder(world)
{
	this->requireOne<HurtboxComponent>();
	this->requireTwo<PlayerComponent>();
	this->requireTwo<HealthComponent>();
}

void HurtboxPlayerResponder::collisionBegan(eid_t e1, eid_t e2, btPersistentManifold* contactManifold)
{
	HurtboxComponent* hurtboxComponent = world.getComponent<HurtboxComponent>(e1);
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(e2);
	HealthComponent* healthComponent = world.getComponent<HealthComponent>(e2);

	std::vector<eid_t>& ignoreEntities = hurtboxComponent->collidedEntities;
	if (std::find(ignoreEntities.begin(), ignoreEntities.end(), e2) != ignoreEntities.end()) {
		return;
	}

	ignoreEntities.push_back(e2);
	healthComponent->health -= hurtboxComponent->damage;
	printf("%d\n", healthComponent->health);
}

void HurtboxPlayerResponder::collisionEnded(eid_t e1, eid_t e2, btPersistentManifold* contactManifold)
{
}
