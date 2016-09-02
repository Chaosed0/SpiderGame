
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

	TransformComponent* cameraTransformComponent = world.getComponent<TransformComponent>(playerComponent->camera);
	CameraComponent* cameraComponent = world.getComponent<CameraComponent>(playerComponent->camera);

	glm::vec3 from = cameraTransformComponent->transform->getPosition();
	glm::vec3 to = from + cameraTransformComponent->transform->getForward() * 1.5f;
	eid_t hitEntity = Util::raycast(this->dynamicsWorld, from, to);
	std::string text;

	TransformComponent* hitEntityTransformComponent = world.getComponent<TransformComponent>(hitEntity);

	if (hitEntityTransformComponent != nullptr) {
		glm::vec2 screenPoint = cameraComponent->camera.worldToScreenPoint(hitEntityTransformComponent->transform->getPosition());
		label->transform.setPosition(glm::vec3(screenPoint + glm::vec2(20.0f, -20.0f), 0.0f));
	}

	if (playerComponent->lastFacedEntity == hitEntity) {
		return;
	}
	playerComponent->lastFacedEntity = hitEntity;

	if (hitEntity == World::NullEntity || world.getEntityName(hitEntity).compare(0, 3, "Gem") != 0) {
		text = "";
	} else {
		text = "[e] to pick up gem";
	}

	label->setText(text);
}