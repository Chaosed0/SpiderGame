
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include <glm/gtx/projection.hpp>

Camera::Camera(float fieldOfView, unsigned int width, unsigned int height, float nearClip, float farClip)
	: fieldOfView(fieldOfView),
	width(width),
	height(height),
	nearClip(nearClip),
	farClip(farClip)
{
	up = glm::vec3(0, 1, 0);
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::inverse(transform.matrix());
}

glm::mat4 Camera::getProjectionMatrix()
{
	return glm::perspective(fieldOfView, (float)width / (float)height, nearClip, farClip);
}

void Camera::lookAt(glm::vec3 position)
{
	glm::quat rotation;
	glm::vec3 direction = glm::normalize(position - transform.getPosition());
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

	transform.setRotation(rotation);
}

void Camera::rotateHorizontalVertical(float horizontal, float vertical)
{
	glm::vec3 direction(
		cos(vertical) * sin(horizontal),
		sin(vertical),
		cos(vertical) * cos(horizontal)
	);

	glm::vec3 right = glm::normalize(glm::cross(direction, up));
	glm::vec3 lookAtUp = glm::cross(right, direction);
	glm::mat4 rotationMatrix = glm::mat4(
		right.x, right.y, right.z, 0.0f,
		lookAtUp.x, lookAtUp.y, lookAtUp.z, 0.0f,
		-direction.x, -direction.y, -direction.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
	transform.setRotation(glm::quat(rotationMatrix));
}
