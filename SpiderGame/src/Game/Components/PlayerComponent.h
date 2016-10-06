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

class PlayerComponent : public Component
{
public:
	PlayerComponent()
		: shooting(false), reloading(false),
		shotTimer(0.0f), reloadTimer(0.0f),
		gunState(GunState_Ready), isDead(false),
		gemCount(0), bulletCount(0), bulletsInGun(0),
		lastFacedEntity(World::NullEntity) { }

	struct Data {
		Data()
			: maxShotDistance(100.0f), shotCooldown(1/30.0f), reloadTime(2.5f),
			gunBarrelOffset(0.0f), maxBulletsInGun(6), shotDamage(100),
			camera(World::NullEntity), gun(World::NullEntity) { }

		eid_t gun;
		float reloadTime;
		float shotCooldown;
		float maxShotDistance;
		unsigned maxBulletsInGun;
		unsigned shotDamage;
		glm::vec3 gunBarrelOffset;

		Prefab shotTracerPrefab;
		Prefab muzzleFlashPrefab;

		AudioClip shotClip;
		AudioClip dryFireClip;
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
	GunState gunState;

	unsigned gemCount;

	unsigned bulletCount;
	unsigned bulletsInGun;

	eid_t lastFacedEntity;
};

class PlayerConstructor : public DefaultComponentConstructor<PlayerComponent> {
	using DefaultComponentConstructor<PlayerComponent>::DefaultComponentConstructor;
};
