#pragma once

#include "Transform.h"

class Camera
{
public:
	Camera(float fieldOfView, unsigned int width, unsigned int height, float nearClip, float farClip);

	void lookAt(glm::vec3 position);
	void rotateHorizontalVertical(float horizontal, float vertical);

	glm::mat4 getViewMatrix();
	glm::mat4 getProjectionMatrix();

	Transform transform;
private:
	float fieldOfView;
	unsigned int width;
	unsigned int height;
	float nearClip;
	float farClip;
	
	glm::vec3 up;
};