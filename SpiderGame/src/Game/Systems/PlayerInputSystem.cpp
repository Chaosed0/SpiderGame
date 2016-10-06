
#include "PlayerInputSystem.h"

#include "Util.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Game/Events/GemCountChangedEvent.h"
#include "Game/Events/BulletCountChangedEvent.h"

static const unsigned bulletPileCount = 12;

PlayerInputSystem::PlayerInputSystem(World& world, Input& input, EventManager& eventManager)
	: System(world), input(input), eventManager(eventManager),
	noclip(false), horizontalRad(0.0f), verticalRad(0.0f)
{
	require<RigidbodyMotorComponent>();
	require<PlayerComponent>();
}

void PlayerInputSystem::updateEntity(float dt, eid_t entity)
{
	Device device = Device_Kbm;
	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(entity);
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(entity);

	if (playerComponent->isDead) {
		return;
	}

	TransformComponent* cameraTransformComponent = world.getComponent<TransformComponent>(playerComponent->data.camera);

	float horizontal = input.getAxis("Horizontal", device);
	float vertical = input.getAxis("Vertical", device);
	float lookHorizontal = input.getAxis("LookHorizontal", device); 
	float lookVertical = input.getAxis("LookVertical", device); 

	horizontalRad -= lookHorizontal * dt;
	verticalRad += lookVertical * dt;
	verticalRad = glm::clamp(verticalRad, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

	cameraTransformComponent->data->setRotation(glm::angleAxis(verticalRad, Util::right));

	if (input.getButtonDown("Use", device)) {
		this->tryActivate(entity, playerComponent);
	}

	rigidbodyMotorComponent->facing = Util::rotateHorizontalVertical(horizontalRad, verticalRad);
	rigidbodyMotorComponent->movement = glm::vec2(vertical, horizontal);
	rigidbodyMotorComponent->jump = input.getButtonDown("Jump", device);
	rigidbodyMotorComponent->noclip = noclip;

	playerComponent->shooting = input.getButtonDown("Fire", device);
	playerComponent->reloading = input.getButtonDown("Reload", device);
}

void PlayerInputSystem::tryActivate(eid_t player, PlayerComponent* playerComponent)
{
	eid_t entity = playerComponent->lastFacedEntity;

	if (entity == World::NullEntity) {
		return;
	}

	std::string name = world.getEntityName(entity);
	if (name.compare(0, 3, "Gem") == 0) {
		playerComponent->gemCount++;
		world.removeEntity(entity);

		GemCountChangedEvent event;
		event.source = player;
		event.newGemCount = playerComponent->gemCount;
		event.oldGemCount = event.newGemCount - 1;
		eventManager.sendEvent(event);
	} else if (name.compare(0, 7, "Bullets") == 0) {
		playerComponent->bulletCount += bulletPileCount;
		world.removeEntity(entity);

		BulletCountChangedEvent event;
		event.source = player;
		event.newBulletCount = playerComponent->bulletCount;
		event.oldBulletCount = event.newBulletCount - 1;
		event.newBulletsInGun = playerComponent->bulletsInGun;
		event.oldBulletsInGun = playerComponent->bulletsInGun;
		eventManager.sendEvent(event);
	}
}

void PlayerInputSystem::setNoclip(bool noclip)
{
	this->noclip = noclip;
}
