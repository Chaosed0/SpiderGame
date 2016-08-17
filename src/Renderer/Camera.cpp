
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

glm::mat4 Camera::getViewMatrixOrtho()
{
	return glm::mat4();
}

glm::mat4 Camera::getProjectionMatrixOrtho()
{
	// Reverse y direction since that's the way 2D applications normally have it
	return glm::ortho(0.0f, (float)width, (float)height, 0.0f);
}

glm::vec2 Camera::worldToScreenPoint(glm::vec3 worldPoint)
{
	glm::vec4 clipSpacePos = this->getProjectionMatrix() * (this->getViewMatrix() * glm::vec4(worldPoint, 1.0f));
	glm::vec3 ndcSpacePos = glm::vec3(clipSpacePos) / clipSpacePos.w;
	ndcSpacePos.y = -ndcSpacePos.y;
	glm::vec2 windowSpacePos = (glm::vec2(ndcSpacePos) + glm::vec2(1.0f)) / 2.0f * glm::vec2(width, height);
	return windowSpacePos;
}
