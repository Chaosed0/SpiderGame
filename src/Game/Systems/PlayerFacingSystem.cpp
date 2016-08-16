
#include "PlayerFacingSystem.h"

#include "Util.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/PlayerComponent.h"

PlayerFacingSystem::PlayerFacingSystem(World& world, btDynamicsWorld* dynamicsWorld, const std::shared_ptr<Label>& label)
	: System(world), dynamicsWorld(dynamicsWorld), label(label)
{
	require<RigidbodyMotorComponent>();
	require<PlayerComponent>();
}

void PlayerFacingSystem::updateEntity(float dt, eid_t entity)
{
	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(entity);
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(entity);

	TransformComponent* cameraTransformComponent = world.getComponent<TransformComponent>(playerComponent->camera);

	glm::vec3 from = cameraTransformComponent->transform.getPosition();
	glm::vec3 to = from + cameraTransformComponent->transform.getRotation() * (Util::forward * playerComponent->maxShotDistance);
	btVector3 btStart(Util::glmToBt(from));
	btVector3 btEnd(Util::glmToBt(to));
	btCollisionWorld::ClosestRayResultCallback rayCallback(btStart, btEnd);
	this->dynamicsWorld->rayTest(btStart, btEnd, rayCallback);

	if (!rayCallback.hasHit()) {
		goto no_hit;
	}

	void* userPtr = rayCallback.m_collisionObject->getUserPointer();
	if (userPtr == nullptr) {
		goto no_hit;
	}

	eid_t hitEntity = *((eid_t*)userPtr);
	if (world.getEntityName(hitEntity).compare(0, 3, "Gem") != 0) {
		goto no_hit;
	}

	label->setText("GEM");
	return;

no_hit:
	label->setText("");
}