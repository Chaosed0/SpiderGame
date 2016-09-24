
#include "Renderer/Camera.h"

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
	farClip(farClip),
	frustumIsDirty(true)
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

void Camera::setInverseViewMatrix(const glm::mat4& inverseViewMatrix)
{
	this->inverseViewMatrix = inverseViewMatrix;
	frustumIsDirty = true;
}

glm::mat4 Camera::getInverseViewMatrix()
{
	return this->inverseViewMatrix;
}

Frustum Camera::getFrustum() const
{
	if (frustumIsDirty) {
		return this->computeFrustum();
	} else {
		return this->cachedFrustum;
	}
}

Frustum Camera::getFrustum()
{
	if (frustumIsDirty) {
		this->cachedFrustum = this->computeFrustum();
	}
	return this->cachedFrustum;
}

Frustum Camera::computeFrustum() const
{
	float nearHeight = 2.0f * tan(fieldOfView / 2.0f) * nearClip;
	float nearWidth = nearHeight * width / height;
	float farHeight = 2.0f * tan(fieldOfView / 2.0f) * farClip;
	float farWidth = farHeight * width / height;

	glm::vec3 cameraPos = glm::vec3(inverseViewMatrix[0][3], inverseViewMatrix[1][3], inverseViewMatrix[2][3]);
	glm::vec3 cameraFwd = glm::vec3(inverseViewMatrix[0][2], inverseViewMatrix[1][2], inverseViewMatrix[2][2]);
	glm::vec3 cameraUp = glm::vec3(inverseViewMatrix[0][1], inverseViewMatrix[1][1], inverseViewMatrix[2][1]);
	glm::vec3 cameraRight = glm::vec3(inverseViewMatrix[0][0], inverseViewMatrix[1][0], inverseViewMatrix[2][0]);

	glm::vec3 nearCenter = cameraPos + cameraFwd * nearClip;
	glm::vec3 nearX = cameraUp * nearHeight / 2.0f;
	glm::vec3 nearY = cameraRight * nearWidth / 2.0f;
	glm::vec3 farCenter = cameraPos + cameraFwd * farClip;
	glm::vec3 farX = cameraUp * farHeight / 2.0f;
	glm::vec3 farY = cameraRight * farWidth / 2.0f;

	FrustumConstructor f;
	f.ntr = nearCenter + nearX + nearY;
	f.ntl = nearCenter + nearX - nearY;
	f.nbr = nearCenter - nearX + nearY;
	f.nbl = nearCenter - nearX - nearY;
	f.ftr = farCenter + farX + farY;
	f.ftl = farCenter + farX - farY;
	f.fbr = farCenter - farX + farY;
	f.fbl = farCenter - farX - farY;

	return Frustum(f);
}