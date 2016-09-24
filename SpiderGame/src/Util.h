#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include "Transform.h"
#include "Framework/World.h"

enum CollisionGroup {
	CollisionGroupDefault = 1,
	CollisionGroupEnemy = 1 << 1,
	CollisionGroupPlayer = 1 << 2,
	CollisionGroupWall = 1 << 3,
	CollisionGroupAll = CollisionGroupDefault | CollisionGroupEnemy | CollisionGroupPlayer | CollisionGroupWall
};

class Util
{
public:
	/*! Remember that this doesn't account for scale - do it yourself in the btShape */
	static btTransform gameToBt(Transform transform);

	static glm::vec3 btToGlm(const btVector3& vec3);

	static glm::quat btToGlm(const btQuaternion& quat);

	static btVector3 glmToBt(const glm::vec3& vec3);

	static btQuaternion glmToBt(const glm::quat& quat);

	static glm::quat lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up);

	static glm::quat rotateHorizontalVertical(float horizontal, float vertical);

	static eid_t raycast(btDynamicsWorld* dynamicsWorld, const glm::vec3& from, const glm::vec3& to, short int collisionGroup = CollisionGroupAll);

	static eid_t raycast(btDynamicsWorld* dynamicsWorld, const btVector3& from, const btVector3& to, short int collisionGroup = CollisionGroupAll);

	static const glm::vec3 up;
	static const glm::vec3 right;
	static const glm::vec3 forward;
};

