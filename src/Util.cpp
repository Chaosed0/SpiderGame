
#include "Util.h"

const glm::vec3 Util::up = glm::vec3(0.0f, 1.0f, 0.0f);
const glm::vec3 Util::right = glm::vec3(1.0f, 0.0f, 0.0f);
const glm::vec3 Util::forward = glm::vec3(0.0f, 0.0f, -1.0f);

btTransform Util::gameToBt(Transform transform)
{
	glm::quat rotation = transform.getRotation();
	glm::vec3 position = transform.getPosition();
	return btTransform(btQuaternion(rotation.x, rotation.y, rotation.z, rotation.w), btVector3(position.x, position.y, position.z));
}

glm::vec3 Util::btToGlm(const btVector3& vec3) {
	return glm::vec3(vec3.x(), vec3.y(), vec3.z());
} 

glm::quat Util::btToGlm(const btQuaternion& quat) {
	return glm::quat(quat.w(), quat.x(), quat.y(), quat.z());
}

btVector3 Util::glmToBt(const glm::vec3& vec3) {
	return btVector3(vec3.x, vec3.y, vec3.z);
} 

btQuaternion Util::glmToBt(const glm::quat& quat) {
	return btQuaternion(quat.x, quat.y, quat.z, quat.w);
}

glm::quat Util::lookAt(glm::vec3 position, glm::vec3 target, glm::vec3 up)
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

glm::quat Util::rotateHorizontalVertical(float horizontal, float vertical)
{
	return glm::quat(glm::vec3(vertical, horizontal, 0.0f));
}

eid_t Util::raycast(btDynamicsWorld* dynamicsWorld, const glm::vec3& from, const glm::vec3& to)
{
	return Util::raycast(dynamicsWorld, Util::glmToBt(from), Util::glmToBt(to));
}

eid_t Util::raycast(btDynamicsWorld* dynamicsWorld, const btVector3& from, const btVector3& to)
{
	btCollisionWorld::ClosestRayResultCallback rayCallback(from, to);
	dynamicsWorld->rayTest(from, to, rayCallback);

	if (!rayCallback.hasHit()) {
		return World::NullEntity;
	}

	void* userPtr = rayCallback.m_collisionObject->getUserPointer();
	if (userPtr == nullptr) {
		return World::NullEntity;
	}

	return *((eid_t*)userPtr);
}
