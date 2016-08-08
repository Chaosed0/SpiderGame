
#include "FollowSystem.h"

#include "Util.h"

#include "Game/Components/TransformComponent.h"
#include "Game/Components/FollowComponent.h"
#include "Game/Components/RigidbodyMotorComponent.h"
#include "Game/Components/CollisionComponent.h"

#include <cmath>

FollowSystem::FollowSystem(World& world, btDynamicsWorld* dynamicsWorld)
	: System(world),
	dynamicsWorld(dynamicsWorld)
{
	require<TransformComponent>();
	require<FollowComponent>();
	require<RigidbodyMotorComponent>();
	require<CollisionComponent>();
}

void FollowSystem::updateEntity(float dt, eid_t entity)
{
	TransformComponent* transformComponent = world.getComponent<TransformComponent>(entity);
	FollowComponent* followComponent = world.getComponent<FollowComponent>(entity);
	RigidbodyMotorComponent* rigidbodyMotorComponent = world.getComponent<RigidbodyMotorComponent>(entity);
	CollisionComponent* collisionComponent = world.getComponent<CollisionComponent>(entity);

	if (followComponent->target == nullptr) {
		return;
	}

	Transform& finalTarget = followComponent->target->transform;

	glm::vec3 from = transformComponent->transform.getPosition();
	glm::vec3 to = finalTarget.getPosition();
	btVector3 btStart(Util::glmToBt(from));
	btVector3 btEnd(Util::glmToBt(to));
	btCollisionWorld::ClosestRayResultCallback rayCallback(btStart, btEnd);
	rayCallback.m_collisionFilterMask = CollisionGroupWall;
	this->dynamicsWorld->rayTest(btStart, btEnd, rayCallback);

	float closestObject = (rayCallback.m_hitPointWorld - btStart).length();
	float distanceToTarget = (btEnd - btStart).length();

	// Bullet reports the hit point as very far away if no contact is found
	if (closestObject >= distanceToTarget) {
		glm::vec3 dir = to - from;
		float angle = atan2f(dir.x, dir.z);
		btQuaternion quat(btVector3(0.0f, 1.0f, 0.0f), angle);
		rigidbodyMotorComponent->facing = Util::btToGlm(quat);
		rigidbodyMotorComponent->movement = glm::vec2(-1.0f, 0.0f);
	} else {
		rigidbodyMotorComponent->movement = glm::vec2(0.0f, 0.0f);
	}
}