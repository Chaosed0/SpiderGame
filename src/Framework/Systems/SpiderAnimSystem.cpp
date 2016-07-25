
#include "SpiderAnimSystem.h"

#include "Framework/Components/CollisionComponent.h"
#include "Framework/Components/ModelRenderComponent.h"
#include "Framework/Components/HealthComponent.h"
#include "Framework/Components/SpiderComponent.h"

#include "Framework/Components/ExpiresComponent.h"

SpiderAnimSystem::SpiderAnimSystem(World& world, Renderer& renderer)
	: System(world),
	renderer(renderer)
{
	require<CollisionComponent>();
	require<ModelRenderComponent>();
	require<HealthComponent>();
	require<SpiderComponent>();
}

void SpiderAnimSystem::updateEntity(float dt, eid_t entity)
{
	CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(entity);
	ModelRenderComponent* modelRenderComponent = world.getComponent<ModelRenderComponent>(entity);
	HealthComponent* healthComponent = world.getComponent<HealthComponent>(entity);
	SpiderComponent* spiderComponent = world.getComponent<SpiderComponent>(entity);

	btVector3 velocity = collisionComponent->body->getLinearVelocity();
	SpiderAnimState newAnimState = spiderComponent->animState;
	if (velocity.length() > 0.01f) {
		newAnimState = SPIDERANIM_MOVING;
	} else {
		newAnimState = SPIDERANIM_IDLE;
	}

	if (healthComponent->health <= 0) {
		newAnimState = SPIDERANIM_DEAD;
	}

	if (newAnimState == spiderComponent->animState) {
		return;
	}

	std::string anim;
	switch(newAnimState) {
	case SPIDERANIM_IDLE:
		anim = "AnimStack::idle";
		break;
	case SPIDERANIM_MOVING:
		anim = "AnimStack::walk";
		break;
	case SPIDERANIM_DEAD:
		anim = "AnimStack::die";
		break;
	}

	if (newAnimState == SPIDERANIM_DEAD) {
		ExpiresComponent* expiresComponent = world.addComponent<ExpiresComponent>(entity);
		expiresComponent->expiryTime = 2.0f;
	}

	bool loop = newAnimState != SPIDERANIM_DEAD;
	renderer.setRenderableAnimation(modelRenderComponent->rendererHandle, anim, loop);
	spiderComponent->animState = newAnimState;
}
