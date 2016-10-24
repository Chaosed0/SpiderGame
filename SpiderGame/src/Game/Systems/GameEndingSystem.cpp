
#include "GameEndingSystem.h"

#include "Game/Components/PlayerComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Events/EndGameEvent.h"

#include "Game/Components/SpiderComponent.h"
#include "Game/Components/SpawnerComponent.h"
#include "Game/Components/ShakeComponent.h"

#include "Util.h"

#include <algorithm>

const float GameEndingSystem::screenShakeTime = 3.0f;
const float GameEndingSystem::gemDefenseTime = 30.0f;
const float GameEndingSystem::blackoutTime = 3.0f;
const float GameEndingSystem::fadeInTime = 2.0f;
const float GameEndingSystem::endRestTime = 2.0f;

GameEndingSystem::GameEndingSystem(World& world, EventManager& eventManager, SoundManager& soundManager)
	: System(world), eventManager(eventManager), soundManager(soundManager)
{
	require<TransformComponent>();
	require<PlayerComponent>();

	eventManager.registerForEvent<GemCountChangedEvent>(std::bind(&GameEndingSystem::onGemCountChanged, this, std::placeholders::_1));
	eventManager.registerForEvent<CollisionEvent>(std::bind(&GameEndingSystem::onCollision, this, std::placeholders::_1));
}

void GameEndingSystem::updateEntity(float dt, eid_t entity)
{
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(entity);

	playerComponent->gameEndTimer += dt;
	if (playerComponent->gameEndState == GameEndState_DefendingGems) {
		if (playerComponent->gameEndTimer >= gemDefenseTime - screenShakeTime) {
		ShakeComponent* cameraShakeComponent = world.getComponent<ShakeComponent>(playerComponent->data.camera);
			if (cameraShakeComponent == nullptr) {
				cameraShakeComponent = world.addComponent<ShakeComponent>(playerComponent->data.camera);
				cameraShakeComponent->data.shakeTime = -1.0f;
				cameraShakeComponent->data.frequency = 1/30.0f;
				cameraShakeComponent->data.amplitude = 0.25f;
				cameraShakeComponent->timer = 0.0f;
				cameraShakeComponent->active = true;
			}
		}

		if (playerComponent->gameEndTimer >= gemDefenseTime) {
			playerComponent->gameEndState = GameEndState_EnteringPortal;

			PrefabConstructionInfo info = PrefabConstructionInfo(Transform());
			world.constructPrefab(playerComponent->data.victoryPortalPrefab, World::NullEntity, &info);
		}
	} else if (playerComponent->gameEndState == GameEndState_Blackout) {
		if (playerComponent->gameEndTimer >= blackoutTime) {
			playerComponent->gameEndState = GameEndState_Fadein;
			playerComponent->gameEndTimer -= blackoutTime;

			// Setup the camera
			std::vector<eid_t> cameras = world.getEntitiesWithComponent<CameraComponent>();
			assert (cameras.size() > 0);
			CameraComponent* oldCameraComponent = world.getComponent<CameraComponent>(cameras[0]);

			eid_t endGameCamera = world.getNewEntity();
			TransformComponent* transformComponent = world.addComponent<TransformComponent>(endGameCamera);
			CameraComponent* cameraComponent = world.addComponent<CameraComponent>(endGameCamera);

			// Copy all the properties of the old camera
			cameraComponent->data = oldCameraComponent->data;

			// Point us toward the sky
			transformComponent->data->setPosition(glm::vec3(0.0f, 100.0f, 0.0f));
			transformComponent->data->setRotation(glm::angleAxis(glm::radians(75.0f), Util::right));

			// Disable the old camera
			oldCameraComponent->isActive = false;
		}
	} else if (playerComponent->gameEndState == GameEndState_Fadein) {
		float alpha = (std::max)(1.0f - playerComponent->gameEndTimer / fadeInTime, 0.0f);
		Material& material = playerComponent->data.blackoutQuad->material;
		material.setProperty("color", MaterialProperty(glm::vec4(0.0f, 0.0f, 0.0f, alpha)));
		if (playerComponent->gameEndTimer >= fadeInTime) {
			playerComponent->gameEndState = GameEndState_Rest;
			playerComponent->gameEndTimer-= fadeInTime;
		}
	} else if (playerComponent->gameEndState == GameEndState_Rest) {
		if (playerComponent->gameEndTimer >= endRestTime) {
			eventManager.sendEvent(VictorySequenceEndedEvent());
			playerComponent->gameEndState = GameEndState_Finished;
			playerComponent->gameEndTimer = 0.0f;
		}
	}
}

void GameEndingSystem::onGemCountChanged(const GemCountChangedEvent& gemCountChangedEvent)
{
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(gemCountChangedEvent.source);
	assert (playerComponent != nullptr);

	bool allGemsPlaced = true;
	for (unsigned i = 0; i < playerComponent->gemStates.size(); i++) {
		if (playerComponent->gemStates[i] != PlayerGemState_Placed) {
			allGemsPlaced = false;
			break;
		}
	}

	if (allGemsPlaced) {
		AllGemsCollectedEvent event;
		event.source = gemCountChangedEvent.source;
		eventManager.sendEvent(event);

		playerComponent->gameEndState = GameEndState_DefendingGems;
		playerComponent->gameEndTimer = 0.0f;
	}
}

void GameEndingSystem::onCollision(const CollisionEvent& collisionEvent)
{
	eid_t player = collisionEvent.e1;
	eid_t other = collisionEvent.e2;
	if (!world.orderEntities(player, other, requiredComponents, ComponentBitmask())) {
		return;
	}

	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(player);

	if (playerComponent->gameEndState != GameEndState_EnteringPortal) {
		return;
	}

	if (world.getEntityName(other).compare("VictoryPortal") != 0) {
		return;
	}

	playerComponent->data.blackoutQuad->isVisible = true;
	playerComponent->gameEndState = GameEndState_Blackout;
	playerComponent->gameEndTimer = 0.0f;
	soundManager.stopAllClips();

	std::vector<eid_t> spiders = world.getEntitiesWithComponent<SpiderComponent>();
	for (unsigned i = 0; i < spiders.size(); i++) {
		world.removeEntity(spiders[i]);
	}

	std::vector<eid_t> spawners = world.getEntitiesWithComponent<SpawnerComponent>();
	for (unsigned i = 0; i < spawners.size(); i++) {
		world.removeEntity(spawners[i]);
	}

	eventManager.sendEvent(VictorySequenceStartedEvent());
}