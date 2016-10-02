#pragma once

#include "Framework/Component.h"
#include "Framework/World.h"
#include "Sound/AudioClip.h"
#include "Renderer/Renderer.h"

enum GunState {
	GunState_Reloading,
	GunState_Ready,
};

class PlayerComponent : public Component
{
public:
	PlayerComponent()
		: shooting(false), reloading(false),
		maxShotDistance(100.0f), shotCooldown(1/30.0f), shotTimer(0.0f), reloadTime(2.5f), reloadTimer(0.0f), shotDamage(10),
		gunBarrelOffset(0.0f), gunState(GunState_Ready),
		gemCount(0), bulletCount(0), bulletsInGun(0), maxBulletsInGun(6),
		lastFacedEntity(World::NullEntity), camera(World::NullEntity), gun(World::NullEntity) { }

	bool shooting;
	bool reloading;

	float reloadTime;
	float reloadTimer;
	float shotCooldown;
	float shotTimer;
	float maxShotDistance;
	GunState gunState;

	unsigned shotDamage;
	unsigned gemCount;

	eid_t gun;
	glm::vec3 gunBarrelOffset;
	Renderer::ModelHandle shotTracerModelHandle;
	Shader tracerShader;
	Renderer::ModelHandle muzzleFlashModelHandle;
	Shader flashShader;
	unsigned bulletCount;
	unsigned maxBulletsInGun;
	unsigned bulletsInGun;

	AudioClip shotClip;
	AudioClip dryFireClip;
	AudioClip hurtClip;
	AudioClip gemPickupClip;

	eid_t lastFacedEntity;
	eid_t camera;
};
