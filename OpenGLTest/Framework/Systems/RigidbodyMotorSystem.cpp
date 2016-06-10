
#include "RigidbodyMotorSystem.h"

#include "Framework/Components/RigidbodyMotorComponent.h"
#include "Framework/Components/CollisionComponent.h"

#include "Util.h"

RigidbodyMotorSystem::RigidbodyMotorSystem()
{
	require<RigidbodyMotorComponent>();
	require<CollisionComponent>();
}

void RigidbodyMotorSystem::updateEntity(float dt, Entity& entity)
{
	RigidbodyMotorComponent* rigidbodyMotorComponent = entity.getComponent<RigidbodyMotorComponent>();
	CollisionComponent* collisionComponent = entity.getComponent<CollisionComponent>();

	btRigidBody* playerBody = collisionComponent->body;
	btVector3 velocity = playerBody->getLinearVelocity();

	if (glm::length(rigidbodyMotorComponent->movement) > glm::epsilon<float>()) {
		glm::vec2 movement = glm::normalize(rigidbodyMotorComponent->movement) * rigidbodyMotorComponent->moveSpeed;
		velocity.setZ(movement.x);
		velocity.setX(movement.y);
		
		btQuaternion playerRotation = playerBody->getWorldTransform().getRotation();
		velocity = velocity.rotate(playerRotation.getAxis(), playerRotation.getAngle());
	} else {
		velocity.setZ(0.0f);
		velocity.setX(0.0f);
	}

	if (rigidbodyMotorComponent->jump && rigidbodyMotorComponent->canJump) {
		velocity.setY(rigidbodyMotorComponent->jumpSpeed);
		rigidbodyMotorComponent->canJump = false;
	}

	playerBody->setLinearVelocity(velocity);
}
