
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

	ModelRenderComponent* gunRenderComponent = world.getComponent<ModelRenderComponent>(playerComponent->gun);

	playerComponent->shotTimer = std::min(playerComponent->shotTimer + dt, playerComponent->shotCooldown);
	playerComponent->reloadTimer = std::min(playerComponent->reloadTimer + dt, playerComponent->reloadTime);

	// Trying to reload && can reload
	if (playerComponent->reloadTimer >= playerComponent->reloadTime &&
		playerComponent->bulletCount > 0 &&
		playerComponent->bulletsInGun != playerComponent->maxBulletsInGun)
	{
		if (playerComponent->reloading && playerComponent->gunState == GunState_Ready) {
			renderer.setRenderableAnimation(gunRenderComponent->rendererHandle, "AnimStack::Gun|Reload", false);
			playerComponent->gunState = GunState_Reloading;
			playerComponent->reloadTimer = 0.0f;
		} else if (playerComponent->gunState == GunState_Reloading) {
			unsigned oldBulletsInGun = playerComponent->bulletsInGun;
			playerComponent->bulletsInGun = (std::min)(playerComponent->bulletCount, playerComponent->maxBulletsInGun);
			playerComponent->bulletCount -= playerComponent->bulletsInGun - oldBulletsInGun;
			playerComponent->gunState = GunState_Ready;

			BulletCountChangedEvent bulletCountEvent;
			bulletCountEvent.source = entity;
			bulletCountEvent.oldBulletCount = playerComponent->bulletCount;
			bulletCountEvent.newBulletCount = playerComponent->bulletCount;
			bulletCountEvent.oldBulletsInGun = oldBulletsInGun;
			bulletCountEvent.newBulletsInGun = playerComponent->bulletsInGun;
			eventManager.sendEvent(bulletCountEvent);
		}
	}

	// Do the shot if shooting
	if (playerComponent->shooting && playerComponent->reloadTimer >= playerComponent->reloadTime) {
		if (playerComponent->bulletsInGun <= 0) {
			ShotEvent event;
			event.source = entity;
			event.actuallyShot = false;
			eventManager.sendEvent(event);
		} else if (playerComponent->shotTimer >= playerComponent->shotCooldown) {
			std::shared_ptr<Transform> transform = transformComponent->transform;
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
			this->createTracer(playerComponent, 0.2f, 75.0f);
			this->createMuzzleFlash(playerComponent, 0.05f);

			/* Animate the gun */
			renderer.setRenderableAnimation(gunRenderComponent->rendererHandle, "AnimStack::Gun|Shoot", false);

			/* Do the raytrace to see if we hit anything */
			TransformComponent* cameraTransformComponent = world.getComponent<TransformComponent>(playerComponent->camera);
			glm::vec3 from = cameraTransformComponent->transform->getWorldPosition();
			glm::vec3 to = from + cameraTransformComponent->transform->getWorldRotation() * (Util::forward * playerComponent->maxShotDistance);
			eid_t hitEntity = Util::raycast(this->dynamicsWorld, from, to);

			HealthComponent* enemyHealthComponent = world.getComponent<HealthComponent>(hitEntity);
			if (enemyHealthComponent == nullptr) {
				return;
			}

			enemyHealthComponent->health -= playerComponent->shotDamage;
		}
		playerComponent->shooting = false;
	}
}

eid_t ShootingSystem::createTracer(PlayerComponent* playerComponent, float expiryTime, float speed)
{
	eid_t line = world.getNewEntity("ShotTracer");
	TransformComponent* transformComponent = world.addComponent<TransformComponent>(line);
	ModelRenderComponent* modelRenderComponent = world.addComponent<ModelRenderComponent>(line);
	ExpiresComponent* expiresComponent = world.addComponent<ExpiresComponent>(line);
	VelocityComponent* velocityComponent = world.addComponent<VelocityComponent>(line);

	TransformComponent* gunTransformComponent = world.getComponent<TransformComponent>(playerComponent->gun);
	glm::vec3 gunBarrelPosition = gunTransformComponent->transform->getWorldPosition() + gunTransformComponent->transform->getWorldRotation() * playerComponent->gunBarrelOffset;
	glm::quat gunBarrelRotation = gunTransformComponent->transform->getWorldRotation();

	transformComponent->transform->setPosition(gunBarrelPosition);
	transformComponent->transform->setRotation(gunBarrelRotation);
	modelRenderComponent->rendererHandle = renderer.getRenderableHandle(playerComponent->shotTracerModelHandle, playerComponent->tracerShader);
	expiresComponent->expiryTime = expiryTime;
	velocityComponent->speed = speed;

	return line;
}

eid_t ShootingSystem::createMuzzleFlash(PlayerComponent* playerComponent, float expiryTime)
{
	eid_t flash = world.getNewEntity("MuzzleFlash");
	TransformComponent* transformComponent = world.addComponent<TransformComponent>(flash);
	ModelRenderComponent* modelRenderComponent = world.addComponent<ModelRenderComponent>(flash);
	ExpiresComponent* expiresComponent = world.addComponent<ExpiresComponent>(flash);

	TransformComponent* gunTransformComponent = world.getComponent<TransformComponent>(playerComponent->gun);
	glm::vec3 gunBarrelPosition = playerComponent->gunBarrelOffset;
	glm::quat randomRotation = glm::angleAxis(randomAngleDistribution(generator), Util::forward);

	transformComponent->transform->setPosition(gunBarrelPosition);
	transformComponent->transform->setRotation(randomRotation);
	transformComponent->transform->setParent(gunTransformComponent->transform);
	modelRenderComponent->rendererHandle = renderer.getRenderableHandle(playerComponent->muzzleFlashModelHandle, playerComponent->flashShader);
	expiresComponent->expiryTime = expiryTime;

	return flash;
}