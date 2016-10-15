
#include "PlayerFacingSystem.h"

#include "Util.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Game/Components/CameraComponent.h"

#include <sstream>

const float PlayerFacingSystem::maxLookDistance = 3.0f;
const glm::vec2 PlayerFacingSystem::facingLabelOffset(20.0f, 20.0f);

PlayerFacingSystem::PlayerFacingSystem(World& world, btDynamicsWorld* dynamicsWorld)
	: System(world), dynamicsWorld(dynamicsWorld)
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
	glm::vec3 to = from + cameraTransformComponent->data->getWorldForward() * maxLookDistance;
	eid_t hitEntity = Util::raycast(this->dynamicsWorld, from, to, CollisionGroupAll ^ CollisionGroupPlayer);
	std::string text;

	TransformComponent* hitEntityTransformComponent = world.getComponent<TransformComponent>(hitEntity);

	if (hitEntityTransformComponent != nullptr) {
		glm::vec2 screenPoint = cameraComponent->data.worldToScreenPoint(hitEntityTransformComponent->data->getWorldPosition());
		playerComponent->data.facingLabel->transform = Transform(glm::vec3(screenPoint + facingLabelOffset, 0.0f)).matrix();
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
	} else if (hitEntityName.compare(0, 4, "Slab") == 0) {
		int gemIndex = -1;
		std::stringstream sstream;
		sstream << hitEntityName;
		sstream.ignore(5);
		sstream >> gemIndex;
		sstream.clear();
		sstream.str("");

		if (gemIndex >= 0 && (unsigned)gemIndex < playerComponent->gemStates.size()) {
			std::string gemColor;
			if (gemIndex == GemColor_Blue) {
				gemColor = "Blue";
			} else if (gemIndex == GemColor_Green) {
				gemColor = "Green";
			} else {
				gemColor = "Red";
			}

			if (playerComponent->gemStates[gemIndex] == PlayerGemState_NotPickedUp) {
				sstream << "Need " << gemColor << " gem";
			} else if(playerComponent->gemStates[gemIndex] == PlayerGemState_PickedUp) {
				sstream << "[e] to place " << gemColor << " gem";
			}
			text = sstream.str();
		}
	} else {
		text = "";
	}

	playerComponent->data.facingLabel->setText(text);
}