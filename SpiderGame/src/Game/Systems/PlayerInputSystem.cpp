
#include "PlayerInputSystem.h"

#include "Util.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Game/Events/GemCountChangedEvent.h"
#include "Game/Events/BulletCountChangedEvent.h"

#include <sstream>

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
		GemComponent* gemComponent = world.getComponent<GemComponent>(entity);
		unsigned gemIndex = gemComponent->data.color;
		if (gemIndex >= 0 && gemIndex < playerComponent->gemStates.size()) {
			playerComponent->gemStates[gemIndex] = PlayerGemState_PickedUp;
		}

		world.removeEntity(entity);
		world.removeEntity(gemComponent->light);

		GemCountChangedEvent event;
		event.source = player;
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
	} else if (name.compare(0, 4, "Slab") == 0) {
		int gemIndex = -1;
		std::stringstream sstream;
		sstream << name;
		sstream.ignore(5);
		sstream >> gemIndex;
		if (gemIndex >= 0 &&
			(unsigned)gemIndex < playerComponent->gemStates.size() &&
			playerComponent->gemStates[gemIndex] == PlayerGemState_PickedUp)
		{
			playerComponent->gemStates[gemIndex] = PlayerGemState_Placed;

			// Force a refresh of the facing label
			playerComponent->lastFacedEntity = World::NullEntity;

			// Create gem atop the slab
			Prefab& gemPrefab = playerComponent->data.gemPrefabs[gemIndex];
			Prefab& gemLightPrefab = playerComponent->data.gemLightPrefabs[gemIndex];
			gemPrefab.setName("PlacedGem");
			gemLightPrefab.setName("PlacedGemLight");

			TransformComponent* slabTransformComponent = world.getComponent<TransformComponent>(entity);
			Transform transform(*slabTransformComponent->data);
			transform.setPosition(transform.getPosition() + glm::vec3(0.0f, 0.2f, 0.0f));
			PrefabConstructionInfo gemInfo = PrefabConstructionInfo(transform);
			eid_t gem = world.constructPrefab(gemPrefab, World::NullEntity, &gemInfo);
			eid_t gemLight = world.constructPrefab(gemLightPrefab, gem);

			GemComponent* gemComponent = world.getComponent<GemComponent>(gem);
			gemComponent->light = gemLight;

			GemCountChangedEvent event;
			event.source = player;
			eventManager.sendEvent(event);
		}
	}
}

void PlayerInputSystem::setNoclip(bool noclip)
{
	this->noclip = noclip;
}
