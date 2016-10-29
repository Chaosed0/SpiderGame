#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Framework/World.h"
#include "Framework/Prefab.h"
#include "Sound/AudioClip.h"

#include "Renderer/Renderer.h"
#include "Renderer/UI/Label.h"
#include "Renderer/UI/UIQuad.h"

#include "Game/Components/GemComponent.h"

enum GunState {
	GunState_Reloading,
	GunState_Ready,
};

enum PlayerGemState {
	PlayerGemState_NotPickedUp,
	PlayerGemState_PickedUp,
	PlayerGemState_Placed,
	PlayerGemState_Count
};

enum GameEndState
{
	GameEndState_NotEnded,
	GameEndState_DefendingGems,
	GameEndState_EnteringPortal,
	GameEndState_Blackout,
	GameEndState_Fadein,
	GameEndState_Rest,
	GameEndState_Finished
};

class PlayerComponent : public Component
{
public:
	PlayerComponent()
		: shooting(false), reloading(false),
		shotTimer(0.0f), reloadTimer(data.reloadTime),
		gunState(GunState_Ready), isDead(false),
		bulletCount(0), bulletsInGun(0), deathRestartTimer(0.0f),
		lastFacedEntity(World::NullEntity),
		gemStates(PlayerGemState_Count, PlayerGemState_NotPickedUp),
		gameEndTimer(0.0f), gameEndState(GameEndState_NotEnded) { }

	struct Data {
		Data()
			: maxShotDistance(100.0f), shotCooldown(1/30.0f), reloadTime(2.5f),
			gunBarrelOffset(0.0f), maxBulletsInGun(6), shotDamage(100), deathRestartTime(4.0f),
			camera(World::NullEntity), gun(World::NullEntity) { }

		eid_t gun;
		float reloadTime;
		float shotCooldown;
		float maxShotDistance;
		float deathRestartTime;
		unsigned maxBulletsInGun;
		unsigned shotDamage;
		glm::vec3 gunBarrelOffset;

		float gemDefenseTime;
		float blackoutTime;
		float fadeInTime;
		float endRestTime;

		Prefab shotTracerPrefab;
		Prefab muzzleFlashPrefab;
		Prefab victoryPortalPrefab;
		std::vector<Prefab> gemPrefabs;
		std::vector<Prefab> gemLightPrefabs;

		AudioClip shotClip;
		AudioClip dryFireClip;
		AudioClip reloadClip;
		AudioClip hurtClip;
		AudioClip gemPickupClip;

		std::shared_ptr<Label> facingLabel;
		std::shared_ptr<UIQuad> blackoutQuad;
		eid_t camera;
	};

	Data data;

	bool shooting;
	bool reloading;
	bool isDead;

	float reloadTimer;
	float shotTimer;
	float deathRestartTimer;
	GunState gunState;

	float gameEndTimer;
	GameEndState gameEndState;

	std::vector<PlayerGemState> gemStates;

	unsigned bulletCount;
	unsigned bulletsInGun;

	eid_t lastFacedEntity;
};

class PlayerConstructor : public DefaultComponentConstructor<PlayerComponent> {
	using DefaultComponentConstructor<PlayerComponent>::DefaultComponentConstructor;
};
