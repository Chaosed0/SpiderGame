#pragma once

#include <btBulletCollisionCommon.h>

#include "Transform.h"

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
	static btTransform gameToBt(Transform transform)
	{
		glm::quat rotation = transform.getRotation();
		glm::vec3 position = transform.getPosition();
		return btTransform(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w), btVector3(position.x, position.y, position.z));
	}

	static glm::vec3 btToGlm(const btVector3& vec3) {
		return glm::vec3(vec3.x(), vec3.y(), vec3.z());
	} 

	static glm::quat btToGlm(const btQuaternion& quat) {
		return glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
	}

	static btVector3 glmToBt(const glm::vec3& vec3) {
		return btVector3(vec3.x, vec3.y, vec3.z);
	} 

	static btQuaternion glmToBt(const glm::quat& quat) {
		return btQuaternion(quat.x, quat.y, quat.z, quat.w);
	}

	static glm::quat lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up)
	{
		glm::quat rotation;
		glm::vec3 direction = glm::normalize(position - target);
		float dot = glm::dot(direction, up);

		if (fabs(dot + 1.0f) < glm::epsilon<float>()) {
			rotation = glm::angleAxis(glm::half_pi<float>(), glm::vec3(0, 0, 1));
		} else if (fabs(dot - 1.0f) < glm::epsilon<float>()) {
			rotation = glm::angleAxis(-glm::half_pi<float>(), glm::vec3(0, 0, 1));
		} else {
			glm::vec3 right = glm::normalize(glm::cross(direction, up));
			glm::vec3 lookAtUp = glm::normalize(glm::cross(right, direction));
			glm::mat4 rotationMatrix = glm::mat4(
				right.x, right.y, right.z, 0.0f,
				lookAtUp.x, lookAtUp.y, lookAtUp.z, 0.0f,
				-direction.x, -direction.y, -direction.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);
			rotation = glm::quat(rotationMatrix);
		}

		return rotation;
	}

	static glm::quat rotateHorizontalVertical(float horizontal, float vertical)
	{
		return glm::quat(glm::vec3(vertical, horizontal, 0.0f));
	}

	static const glm::vec3 up;
	static const glm::vec3 right;
	static const glm::vec3 forward;
};

