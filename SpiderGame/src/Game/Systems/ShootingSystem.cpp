
#include "ShootingSystem.h"

#include <algorithm>
#include <vector>

#include "Util.h"

#include "Renderer/Renderer.h"
#include "Renderer/Model.h"
#include "Renderer/Box.h"

#include "Game/Components/PlayerComponent.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/CollisionComponent.h"
#include "Game/Components/HealthComponent.h"

#include "Game/Components/ExpiresComponent.h"
#include "Game/Components/ModelRenderComponent.h"
#include "Game/Components/VelocityComponent.h"

#include "Game/Events/ShotEvent.h"
#include "Game/Events/BulletCountChangedEvent.h"

ShootingSystem::ShootingSystem(World& world, btDynamicsWorld* dynamicsWorld, Renderer& renderer, EventManager& eventManager, std::default_random_engine& generator)
	: System(world),
	dynamicsWorld(dynamicsWorld),
	eventManager(eventManager),
	renderer(renderer),
	generator(generator),
	randomAngleDistribution(0.0f, glm::two_pi<float>())
{
	require<PlayerComponent>();
	require<TransformComponent>();
	require<RigidbodyMotorComponent>();
}

void ShootingSystem::updateEntity(float dt, eid_t entity)
{
	RigidbodyMotorComponent* rigidbodyMotorComponent(world.getComponent<RigidbodyMotorComponent>(entity));
	TransformComponent* transformComponent(world.getComponent<TransformComponent>(entity));
	PlayerComponent* playerComponent(world.getComponent<PlayerComponent>(entity));

	ModelRenderComponent* gunRenderComponent = world.getComponent<ModelRenderComponent>(playerComponent->data.gun);

	playerComponent->shotTimer = std::min(playerComponent->shotTimer + dt, playerComponent->data.shotCooldown);
	playerComponent->reloadTimer = std::min(playerComponent->reloadTimer + dt, playerComponent->data.reloadTime);

	// Trying to reload && can reload
	if (playerComponent->reloadTimer >= playerComponent->data.reloadTime &&
		playerComponent->bulletCount > 0 &&
		playerComponent->bulletsInGun != playerComponent->data.maxBulletsInGun)
	{
		if (playerComponent->reloading && playerComponent->gunState == GunState_Ready) {
			renderer.setRenderableAnimation(gunRenderComponent->rendererHandle, "AnimStack::Gun|Reload", false);
			playerComponent->gunState = GunState_Reloading;
			playerComponent->reloadTimer = 0.0f;

			ReloadStartEvent reloadEvent;
			reloadEvent.source = entity;
			eventManager.sendEvent(reloadEvent);
		} else if (playerComponent->gunState == GunState_Reloading) {
			unsigned oldBulletsInGun = playerComponent->bulletsInGun;
			playerComponent->bulletsInGun = (std::min)(playerComponent->bulletCount + oldBulletsInGun, playerComponent->data.maxBulletsInGun);
			playerComponent->bulletCount -= playerComponent->bulletsInGun - oldBulletsInGun;
			playerComponent->gunState = GunState_Ready;

			BulletCountChangedEvent bulletCountEvent;
			bulletCountEvent.source = entity;
			bulletCountEvent.oldBulletCount = playerComponent->bulletCount;
			bulletCountEvent.newBulletCount = playerComponent->bulletCount;
			bulletCountEvent.oldBulletsInGun = oldBulletsInGun;
			bulletCountEvent.newBulletsInGun = playerComponent->bulletsInGun;
			eventManager.sendEvent(bulletCountEvent);

			ReloadEndEvent reloadEvent;
			reloadEvent.source = entity;
			eventManager.sendEvent(reloadEvent);
		}
	}

	// Do the shot if shooting
	if (playerComponent->shooting && playerComponent->reloadTimer >= playerComponent->data.reloadTime) {
		if (playerComponent->bulletsInGun <= 0) {
			ShotEvent event;
			event.source = entity;
			event.actuallyShot = false;
			eventManager.sendEvent(event);
		} else if (playerComponent->shotTimer >= playerComponent->data.shotCooldown) {
			std::shared_ptr<Transform> transform = transformComponent->data;
			playerComponent->shotTimer = 0.0f;

			/* Fire off an event to let people know we shot a bullet */
			ShotEvent event;
			event.source = entity;
			event.actuallyShot = true;
			eventManager.sendEvent(event);

			/* Decrement the player's bullet count */
			--playerComponent->bulletsInGun;
			BulletCountChangedEvent bulletCountEvent;
			bulletCountEvent.source = entity;
			bulletCountEvent.oldBulletCount = playerComponent->bulletCount;
			bulletCountEvent.newBulletCount = playerComponent->bulletCount;
			bulletCountEvent.oldBulletsInGun = playerComponent->bulletsInGun + 1;
			bulletCountEvent.newBulletsInGun = playerComponent->bulletsInGun;
			eventManager.sendEvent(bulletCountEvent);

			/* Create shot FX */
			this->createTracer(playerComponent);
			this->createMuzzleFlash(playerComponent);

			/* Animate the gun */
			renderer.setRenderableAnimation(gunRenderComponent->rendererHandle, "AnimStack::Gun|Shoot", false);

			/* Do the raytrace to see if we hit anything */
			TransformComponent* cameraTransformComponent = world.getComponent<TransformComponent>(playerComponent->data.camera);
			glm::vec3 from = cameraTransformComponent->data->getWorldPosition();
			glm::vec3 to = from + cameraTransformComponent->data->getWorldRotation() * (Util::forward * playerComponent->data.maxShotDistance);
			eid_t hitEntity = Util::raycast(this->dynamicsWorld, from, to);

			HealthComponent* enemyHealthComponent = world.getComponent<HealthComponent>(hitEntity);
			if (enemyHealthComponent == nullptr) {
				return;
			}

			enemyHealthComponent->data.health -= playerComponent->data.shotDamage;
		}
		playerComponent->shooting = false;
	}
}

eid_t ShootingSystem::createTracer(PlayerComponent* playerComponent)
{
	TransformComponent* gunTransformComponent = world.getComponent<TransformComponent>(playerComponent->data.gun);
	glm::vec3 gunBarrelPosition = gunTransformComponent->data->getWorldPosition() + gunTransformComponent->data->getWorldRotation() * playerComponent->data.gunBarrelOffset;
	glm::quat gunBarrelRotation = gunTransformComponent->data->getWorldRotation();
	PrefabConstructionInfo info(Transform(gunBarrelPosition, gunBarrelRotation));

	return world.constructPrefab(playerComponent->data.shotTracerPrefab, World::NullEntity, &info);
}

eid_t ShootingSystem::createMuzzleFlash(PlayerComponent* playerComponent)
{
	glm::vec3 gunBarrelPosition = playerComponent->data.gunBarrelOffset;
	glm::quat randomRotation = glm::angleAxis(randomAngleDistribution(generator), Util::forward);
	PrefabConstructionInfo info(Transform(gunBarrelPosition, randomRotation));

	return world.constructPrefab(playerComponent->data.muzzleFlashPrefab, playerComponent->data.gun, &info);
}
