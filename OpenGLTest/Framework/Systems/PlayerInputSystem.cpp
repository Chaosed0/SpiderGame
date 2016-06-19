
#include "PlayerInputSystem.h"

#include "Util.h"
#include "Framework/Components/RigidbodyMotorComponent.h"
#include "Framework/Components/PlayerComponent.h"

PlayerInputSystem::PlayerInputSystem()
{
	require<RigidbodyMotorComponent>();
	require<PlayerComponent>();
}

void PlayerInputSystem::updateEntity(float dt, Entity& entity)
{
	RigidbodyMotorComponent* rigidbodyMotorComponent = entity.getComponent<RigidbodyMotorComponent>();

	glm::vec2 movement(0.0f,0.0f);
	if (forward) {
		movement.x += 1.0f;
	}
	if (back) {
		movement.x -= 1.0f;
	}
	if (right) {
		movement.y += 1.0f;
	}
	if (left) {
		movement.y -= 1.0f;
	}

	rigidbodyMotorComponent->facing = Util::rotateHorizontalVertical(horizontal, vertical, glm::vec3(0.0f, 1.0f, 0.0f));
	rigidbodyMotorComponent->movement = movement;
	rigidbodyMotorComponent->jump = jump;
	rigidbodyMotorComponent->noclip = noclip;
	jump = false;
}

void PlayerInputSystem::startMoving(glm::vec2 movement)
{
	if (movement.x > 0) {
		forward = true;
	} else if (movement.x < 0) {
		back = true;
	} else if (movement.y > 0) {
		left = true;
	} else if (movement.y < 0) {
		right = true;
	}
}
void PlayerInputSystem::stopMoving(glm::vec2 movement)
{
	if (movement.x > 0) {
		forward = false;
	} else if (movement.x < 0) {
		back = false;
	} else if (movement.y > 0) {
		left = false;
	} else if (movement.y < 0) {
		right = false;
	}
}

void PlayerInputSystem::startJump()
{
	jump = true;
}

void PlayerInputSystem::setNoclip(bool noclip)
{
	this->noclip = noclip;
}

void PlayerInputSystem::setHorizontalVerticalRotation(float horizontal, float vertical)
{
	this->horizontal = horizontal;
	this->vertical = vertical;
}
