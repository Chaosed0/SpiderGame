
#include "GameEndingSystem.h"

#include "Game/Components/PlayerComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Events/EndGameEvent.h"

#include "Game/Components/SpiderComponent.h"
#include "Game/Components/SpawnerComponent.h"
#include "Game/Components/ShakeComponent.h"
#include "Game/Components/AudioSourceComponent.h"

#include "Util.h"

#include <algorithm>

GameEndingSystem::GameEndingSystem(World& world, EventManager& eventManager, SoundManager& soundManager)
	: System(world), eventManager(eventManager), soundManager(soundManager)
{
	require<TransformComponent>();
	require<PlayerComponent>();

	eventManager.registerForEvent<GemCountChangedEvent>(std::bind(&GameEndingSystem::onGemCountChanged, this, std::placeholders::_1));
	eventManager.registerForEvent<GemLightOnEvent>(std::bind(&GameEndingSystem::onGemLightOn, this, std::placeholders::_1));
	eventManager.registerForEvent<CollisionEvent>(std::bind(&GameEndingSystem::onCollision, this, std::placeholders::_1));
}

void GameEndingSystem::updateEntity(float dt, eid_t entity)
{
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(entity);

	playerComponent->gameEndTimer += dt;
	float timer = playerComponent->gameEndTimer;

	PlayerComponent::Data& playerData = playerComponent->data;

	if (playerComponent->gameEndState == GameEndState_DefendingGems) {
		if (timer >= playerData.gemDefenseTime) {
			playerComponent->gameEndState = GameEndState_EnteringPortal;

			PrefabConstructionInfo info = PrefabConstructionInfo(Transform());
			eid_t portal = world.constructPrefab(playerComponent->data.victoryPortalPrefab, World::NullEntity, &info);
			playerComponent->gameEndTimer -= playerData.gemDefenseTime;

			eid_t gameEndSoundSource = world.getEntityWithName("EndGameSoundSource");
			assert(gameEndSoundSource != World::NullEntity);
			AudioSourceComponent* audioSourceComponent = world.getComponent<AudioSourceComponent>(gameEndSoundSource);
			soundManager.setSourceVolume(audioSourceComponent->sourceHandle, 1.0f);
		}
	} else if (playerComponent->gameEndState == GameEndState_EnteringPortal) {
		float alpha = (std::max)(1.0f - timer / playerData.whiteoutTime, 0.0f);
		Material& material = playerComponent->data.blackoutQuad->material;
		material.setProperty("color", MaterialProperty(glm::vec4(1.0f, 1.0f, 1.0f, alpha)));
		playerComponent->data.blackoutQuad->isVisible = true;

		std::vector<eid_t> gems = world.getEntitiesWithComponent<GemComponent>();
		for (unsigned i = 0; i < gems.size(); i++) {
			GemComponent* gemComponent = world.getComponent<GemComponent>(gems[i]);
			gemComponent->data.state = GemState_ShouldFree;
		}
	} else if (playerComponent->gameEndState == GameEndState_Blackout) {
		if (timer >= playerData.blackoutTime) {
			playerComponent->gameEndState = GameEndState_Fadein;
			playerComponent->gameEndTimer -= playerData.blackoutTime;

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

			// Play the outdoors soundscape
			AudioSourceComponent* audioSourceComponent = world.getComponent<AudioSourceComponent>(entity);
			if (audioSourceComponent->sourceHandle != nullptr) {
				soundManager.playClipAtSource(playerComponent->data.windClip, audioSourceComponent->sourceHandle);
			}
		}
	} else if (playerComponent->gameEndState == GameEndState_Fadein) {
		float alpha = (std::max)(1.0f - timer / playerData.fadeInTime, 0.0f);
		Material& material = playerComponent->data.blackoutQuad->material;
		material.setProperty("color", MaterialProperty(glm::vec4(0.0f, 0.0f, 0.0f, alpha)));
		if (timer >= playerData.fadeInTime) {
			playerComponent->gameEndState = GameEndState_Rest;
			playerComponent->gameEndTimer -= playerData.fadeInTime;
		}
	} else if (playerComponent->gameEndState == GameEndState_Rest) {
		if (timer >= playerData.endRestTime) {
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

		eid_t gameEndSoundSource = world.getEntityWithName("EndGameSoundSource");
		assert(gameEndSoundSource != World::NullEntity);
		AudioSourceComponent* audioSourceComponent = world.getComponent<AudioSourceComponent>(gameEndSoundSource);
		soundManager.setSourceVolume(audioSourceComponent->sourceHandle, 0.25f);
		soundManager.playClipAtSource(playerComponent->data.portalClip, audioSourceComponent->sourceHandle, true);
	}
}

void GameEndingSystem::onGemLightOn(const GemLightOnEvent& gemLightOnEvent) {
	std::vector<eid_t> gems = world.getEntitiesWithComponent<GemComponent>();
	int lightsOn = 0;

	for (unsigned i = 0; i < gems.size(); i++) {
		GemComponent* gemComponent = world.getComponent<GemComponent>(gems[i]);
		if (gemComponent->data.lightState == GemLightState_Max) {
			++lightsOn;
		}
	}
	
	eid_t gameEndSoundSource = world.getEntityWithName("EndGameSoundSource");
	assert (gameEndSoundSource != World::NullEntity);

	AudioSourceComponent* audioSourceComponent = world.getComponent<AudioSourceComponent>(gameEndSoundSource);
	soundManager.setSourceVolume(audioSourceComponent->sourceHandle, 0.25f + 0.75f / gems.size() * lightsOn);
}

void GameEndingSystem::onCollision(const CollisionEvent& collisionEvent)
{
	if (collisionEvent.type != CollisionResponseType_Began) {
		return;
	}

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

	Material& material = playerComponent->data.blackoutQuad->material;
	material.setProperty("color", MaterialProperty(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)));
	playerComponent->gameEndState = GameEndState_Blackout;
	playerComponent->gameEndTimer = 0.0f;
	soundManager.stopAllClips();

	AudioSourceComponent* audioSourceComponent = world.getComponent<AudioSourceComponent>(player);
	soundManager.playClipAtSource(playerComponent->data.portalEnterClip, audioSourceComponent->sourceHandle);

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