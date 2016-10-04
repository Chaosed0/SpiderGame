
#include "PlayerFacingSystem.h"

#include "Util.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Game/Components/CameraComponent.h"

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

	TransformComponent* cameraTransformComponent = world.getComponent<TransformComponent>(playerComponent->data.camera);
	CameraComponent* cameraComponent = world.getComponent<CameraComponent>(playerComponent->data.camera);

	glm::vec3 from = cameraTransformComponent->data->getWorldPosition();
	glm::vec3 to = from + cameraTransformComponent->data->getWorldForward() * 2.0f;
	eid_t hitEntity = Util::raycast(this->dynamicsWorld, from, to, CollisionGroupAll ^ CollisionGroupPlayer);
	std::string text;

	TransformComponent* hitEntityTransformComponent = world.getComponent<TransformComponent>(hitEntity);

	if (hitEntityTransformComponent != nullptr) {
		glm::vec2 screenPoint = cameraComponent->data.worldToScreenPoint(hitEntityTransformComponent->data->getWorldPosition());
		label->transform = Transform(glm::vec3(screenPoint + glm::vec2(20.0f, -20.0f), 0.0f)).matrix();
	}

	if (playerComponent->lastFacedEntity == hitEntity) {
		return;
	}
	playerComponent->lastFacedEntity = hitEntity;

	std::string hitEntityName = (hitEntity == World::NullEntity ? "" : world.getEntityName(hitEntity));
	if (hitEntityName.compare(0, 3, "Gem") == 0) {
		text = "[e] to pick up gem";
	} else if (hitEntityName.compare(0, 7, "Bullets") == 0) {
		text = "[e] to pick up bullets";
	} else {
		text = "";
	}

	label->setText(text);
}