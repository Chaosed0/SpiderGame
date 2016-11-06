
#include "PlayerDeathSystem.h"

#include "Game/Components/PlayerComponent.h"
#include "Game/Events/GameEvents.h"

PlayerDeathSystem::PlayerDeathSystem(World& world, EventManager& eventManager)
	: System(world),
	eventManager(eventManager)
{
	require<PlayerComponent>();
}

void PlayerDeathSystem::updateEntity(float dt, eid_t entity)
{
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(entity);

	if (playerComponent->isDead == false) {
		return;
	}

	playerComponent->deathRestartTimer += dt;
	if (playerComponent->deathRestartTimer >= playerComponent->data.deathRestartTime) {
		RestartEvent event;
		event.source = entity;
		eventManager.sendEvent(event);
	}
}
