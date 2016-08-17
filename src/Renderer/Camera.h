#pragma once

#include "Transform.h"

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

	glm::mat4 inverseViewMatrix;
private:
	float fieldOfView;
	unsigned int width;
	unsigned int height;
	float nearClip;
	float farClip;
};