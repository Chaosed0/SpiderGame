#pragma once

#include "Framework/Component.h"
#include "Framework/World.h"
#include "Sound/AudioClip.h"
#include "Renderer/Renderer.h"

class PlayerComponent : public Component
{
public:
	PlayerComponent()
		: shooting(false),
		maxShotDistance(100.0f), shotCooldown(1/30.0f), shotTimer(0.0f), shotDamage(10),
		gunBarrelOffset(0.0f), gunKickTime(0.025f), gunReturnTime(0.3f), gunKickAngle(glm::radians(70.0f)), gunRecoilTimer(0.0f),
		gemCount(0), gunRecoiling(false), bulletCount(0),
		lastFacedEntity(World::NullEntity), camera(World::NullEntity), gun(World::NullEntity) { }

	bool shooting;
	float shotCooldown;
	float shotTimer;
	float maxShotDistance;
	unsigned shotDamage;
	unsigned gemCount;

	eid_t gun;
	glm::vec3 gunBarrelOffset;
	Renderer::ModelHandle shotTracerModelHandle;
	Shader tracerShader;
	Renderer::ModelHandle muzzleFlashModelHandle;
	Shader flashShader;
	float gunKickTime;
	float gunReturnTime;
	float gunKickAngle;
	float gunKickDistance;
	float gunRecoilTimer;
	bool gunRecoiling;
	unsigned bulletCount;

	AudioClip shotClip;
	AudioClip dryFireClip;
	AudioClip hurtClip;
	AudioClip gemPickupClip;

	eid_t lastFacedEntity;
	eid_t camera;
};
