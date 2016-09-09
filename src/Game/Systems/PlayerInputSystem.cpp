
#include "PlayerInputSystem.h"

#include "Util.h"
#include "Game/Components/TransformComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/PlayerComponent.h"
#include "Game/Events/GemCountChangedEvent.h"

PlayerInputSystem::PlayerInputSystem(World& world, Input& input, EventManager& eventManager)
	: System(world), input(input), eventManager(eventManager),
	noclip(false), horizontalRad(0.0f), verticalRad(0.0f)
{
	require<RigidbodyMotorComponent>();
	require<PlayerComponent>();
}

void PlayerInputSystem::updateEntity(float dt, eid_t entity)
{
	Device device = Device_Controller0;
	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(entity);
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(entity);

	TransformComponent* cameraTransformComponent = world.getComponent<TransformComponent>(playerComponent->camera);

	float horizontal = input.getAxis("Horizontal", device);
	float vertical = input.getAxis("Vertical", device);
	float lookHorizontal = input.getAxis("LookHorizontal", device); 
	float lookVertical = input.getAxis("LookVertical", device); 

	horizontalRad -= lookHorizontal * dt;
	verticalRad += lookVertical * dt;
	verticalRad = glm::clamp(verticalRad, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);

	cameraTransformComponent->transform->setRotation(glm::angleAxis(verticalRad, glm::vec3(1.0f, 0.0f, 0.0f)));

	if (input.getButtonDown("Use", device)) {
		this->tryActivate(playerComponent);
	}

	rigidbodyMotorComponent->facing = Util::rotateHorizontalVertical(horizontalRad, verticalRad);
	rigidbodyMotorComponent->movement = glm::vec2(vertical, horizontal);
	rigidbodyMotorComponent->jump = input.getButtonDown("Jump", device);
	rigidbodyMotorComponent->noclip = noclip;

	playerComponent->shooting = input.getButton("Fire", device);
}

void PlayerInputSystem::tryActivate(PlayerComponent* playerComponent)
{
	eid_t entity = playerComponent->lastFacedEntity;

	if (entity == World::NullEntity) {
		return;
	}

	if (world.getEntityName(entity).compare(0, 3, "Gem") == 0) {
		playerComponent->gemCount++;
		world.removeEntity(entity);

		GemCountChangedEvent event;
		event.newGemCount = playerComponent->gemCount;
		event.oldGemCount = event.newGemCount - 1;
		eventManager.sendEvent(event);
	}
}

void PlayerInputSystem::setNoclip(bool noclip)
{
	this->noclip = noclip;
}
