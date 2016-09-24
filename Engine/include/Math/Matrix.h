#pragma once

#include <glm/glm.hpp>

struct mat4
{
	/*float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;*/
	float m11, m21, m31, m41;
	float m12, m22, m32, m42;
	float m13, m23, m33, m43;
	float m14, m24, m34, m44;

	mat4 operator*(const mat4 other);
	mat4(const glm::mat4& matrix);
	mat4();
	mat4(float m11, float m21, float m31, float m41,
		float m12, float m22, float m32, float m42,
		float m13, float m23, float m33, float m43,
		float m14, float m24, float m34, float m44);
	glm::mat4 toGlm();
};
