
#include "ExpiresSystem.h"

#include "Framework/Components/ExpiresComponent.h"

ExpiresSystem::ExpiresSystem(World& world)
	: System(world)
{
	require<ExpiresComponent>();
}

void ExpiresSystem::updateEntity(float dt, eid_t entity)
{
	ExpiresComponent* expiresComponent = world.getComponent<ExpiresComponent>(entity);

	expiresComponent->timer += dt;
	if (expiresComponent->timer >= expiresComponent->expiryTime) {
		world.removeEntity(entity);
	}
}
