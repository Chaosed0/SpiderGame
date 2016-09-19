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
		gunBarrelOffset(0.0f), gunKickTime(0.05f), gunReturnTime(0.3f), gunKickAngle(glm::quarter_pi<float>()), gunRecoilTimer(0.0f),
		gemCount(0), gunRecoiling(false),
		lastFacedEntity(World::NullEntity), camera(World::NullEntity) { }

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
	float gunRecoilTimer;
	bool gunRecoiling;

	AudioClip shotClip;
	AudioClip hurtClip;
	AudioClip gemPickupClip;

	eid_t lastFacedEntity;
	eid_t camera;
};
