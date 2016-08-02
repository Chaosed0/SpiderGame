
#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/orthonormalize.hpp>
#include <glm/gtx/projection.hpp>

Camera::Camera() { }

Camera::Camera(float fieldOfView, unsigned int width, unsigned int height, float nearClip, float farClip)
	: fieldOfView(fieldOfView),
	width(width),
	height(height),
	nearClip(nearClip),
	farClip(farClip)
{
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::inverse(inverseViewMatrix);
}

glm::mat4 Camera::getProjectionMatrix()
{
	return glm::perspective(fieldOfView, (float)width / (float)height, nearClip, farClip);
}

glm::mat4 Camera::getProjectionMatrixOrtho()
{
	return glm::ortho(0, (int)width, 0, (int)height);
}
