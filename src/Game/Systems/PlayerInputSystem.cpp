
#include "PlayerInputSystem.h"

#include "Util.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/PlayerComponent.h"

PlayerInputSystem::PlayerInputSystem(World& world)
	: System(world),
	forward(false), back(false), left(false), right(false),
	jump(false), noclip(false), shooting(false),
	horizontalRad(0.0f), verticalRad(0.0f),
	rotateHorizontal(0.0f), rotateVertical(0.0f)
{
	require<RigidbodyMotorComponent>();
	require<PlayerComponent>();
}

void PlayerInputSystem::updateEntity(float dt, eid_t entity)
{
	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(entity);
	PlayerComponent* playerComponent = world.getComponent<PlayerComponent>(entity);

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

	if (std::fabs(rotateHorizontal) > 0) {
		horizontalRad += rotateHorizontal * dt * 0.2f;
	}
	if (std::fabs(rotateVertical) > 0.0f) {
		verticalRad += rotateVertical * dt * 0.2f;
		verticalRad = glm::clamp(verticalRad, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f);
	}

	rigidbodyMotorComponent->facing = Util::rotateHorizontalVertical(horizontalRad, verticalRad);
	rigidbodyMotorComponent->movement = movement;
	rigidbodyMotorComponent->jump = jump;
	rigidbodyMotorComponent->noclip = noclip;
	jump = false;

	playerComponent->shooting = shooting;
	rotateHorizontal = rotateVertical = 0.0f;
}

void PlayerInputSystem::startMoving(glm::vec2 movement)
{
	if (movement.x > 0) {
		forward = true;
	} else if (movement.x < 0) {
		back = true;
	} else if (movement.y > 0) {
		right = true;
	} else if (movement.y < 0) {
		left = true;
	}
}
void PlayerInputSystem::stopMoving(glm::vec2 movement)
{
	if (movement.x > 0) {
		forward = false;
	} else if (movement.x < 0) {
		back = false;
	} else if (movement.y > 0) {
		right = false;
	} else if (movement.y < 0) {
		left = false;
	}
}

void PlayerInputSystem::startJump()
{
	jump = true;
}

void PlayerInputSystem::setShooting(bool shooting)
{
	this->shooting = shooting;
}

void PlayerInputSystem::setNoclip(bool noclip)
{
	this->noclip = noclip;
}

void PlayerInputSystem::rotateCamera(float horizontal, float vertical)
{
	// mouse right is positive, but clockwise rotation is negative
	this->rotateHorizontal -= horizontal;

	// mouse down is positive, but clockwise rotation is negative
	this->rotateVertical -= vertical;
}

float PlayerInputSystem::getCameraVertical()
{
	return this->verticalRad;
}