#pragma once

#include <btBulletCollisionCommon.h>

#include "Transform.h"

class Util
{
public:
	/*! Remember that this doesn't account for scale - do it yourself in the btShape */
	static btTransform transformToBt(Transform transform)
	{
		glm::quat rotation = transform.getRotation();
		glm::vec3 position = transform.getPosition();
		return btTransform(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w), btVector3(position.x, position.y, position.z));
	}
};
