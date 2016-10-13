#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Framework/World.h"
#include "Framework/Prefab.h"
#include "Sound/AudioClip.h"
#include "Renderer/Renderer.h"
#include "Renderer/UI/Label.h"

enum GunState {
	GunState_Reloading,
	GunState_Ready,
};

enum GemState {
	GemState_NotPickedUp,
	GemState_PickedUp,
	GemState_Placed
};

class PlayerComponent : public Component
{
public:
	PlayerComponent()
		: shooting(false), reloading(false),
		shotTimer(0.0f), reloadTimer(data.reloadTime),
		gunState(GunState_Ready), isDead(false),
		gemStates(3), bulletCount(0), bulletsInGun(0), deathRestartTimer(0.0f),
		lastFacedEntity(World::NullEntity) { }

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

		Prefab shotTracerPrefab;
		Prefab muzzleFlashPrefab;

		AudioClip shotClip;
		AudioClip dryFireClip;
		AudioClip reloadClip;
		AudioClip hurtClip;
		AudioClip gemPickupClip;

		std::shared_ptr<Label> facingLabel;
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

	std::vector<GemState> gemStates;

	unsigned bulletCount;
	unsigned bulletsInGun;

	eid_t lastFacedEntity;
};

class PlayerConstructor : public DefaultComponentConstructor<PlayerComponent> {
	using DefaultComponentConstructor<PlayerComponent>::DefaultComponentConstructor;
};
