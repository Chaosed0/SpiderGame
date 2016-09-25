#pragma once

#include "Frustum.h"

class Camera
{
public:
	Camera();
	Camera(float fieldOfView, unsigned int width, unsigned int height, float nearClip, float farClip);

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	glm::mat4 getViewMatrixOrtho();
	glm::mat4 getProjectionMatrixOrtho();

	glm::vec2 worldToScreenPoint(glm::vec3 worldPoint);

	void setInverseViewMatrix(const glm::mat4& inverseViewMatrix);
	glm::mat4 getInverseViewMatrix();

	Frustum getFrustum();
	Frustum getFrustum() const;
private:
	float fieldOfView;
	unsigned int width;
	unsigned int height;
	float nearClip;
	float farClip;

	glm::mat4 inverseViewMatrix;

	Frustum computeFrustum() const;

	Frustum cachedFrustum;
	bool frustumIsDirty;
};