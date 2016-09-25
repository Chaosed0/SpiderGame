
#include "Math/Matrix.h"

mat4::mat4()
{
	this->m11 = 1.0f; this->m21 = this->m31 = this->m41 = 0.0f;
	this->m22 = 1.0f; this->m12 = this->m32 = this->m42 = 0.0f;
	this->m33 = 1.0f; this->m13 = this->m23 = this->m43 = 0.0f;
	this->m44 = 1.0f; this->m14 = this->m24 = this->m34 = 0.0f;
}

mat4::mat4(float m11, float m21, float m31, float m41,
	float m12, float m22, float m32, float m42,
	float m13, float m23, float m33, float m43,
	float m14, float m24, float m34, float m44)
	: m11(m11), m21(m21), m31(m31), m41(m41),
	m12(m12), m22(m22), m32(m32), m42(m42),
	m13(m13), m23(m23), m33(m33), m43(m43),
	m14(m14), m24(m24), m34(m34), m44(m44)
{ }

mat4::mat4(const glm::mat4& matrix)
{
	memcpy(this, &matrix[0][0], sizeof(float)*16);
}

glm::mat4 mat4::toGlm()
{
	glm::mat4 matrix = glm::mat4();
	memcpy(&matrix[0][0], this, sizeof(float)*16);
	return matrix;
}

mat4 mat4::operator*(const mat4 other)
{
	mat4 retval;
	retval.m11 = this->m11 * other.m11 + this->m12 * other.m21 + this->m13 * other.m31 + this->m14 * other.m41;
	retval.m12 = this->m11 * other.m12 + this->m12 * other.m22 + this->m13 * other.m32 + this->m14 * other.m42;
	retval.m13 = this->m11 * other.m13 + this->m12 * other.m23 + this->m13 * other.m33 + this->m14 * other.m43;
	retval.m14 = this->m11 * other.m14 + this->m12 * other.m24 + this->m13 * other.m34 + this->m14 * other.m44;

	retval.m21 = this->m21 * other.m11 + this->m22 * other.m21 + this->m23 * other.m31 + this->m24 * other.m41;
	retval.m22 = this->m21 * other.m12 + this->m22 * other.m22 + this->m23 * other.m32 + this->m24 * other.m42;
	retval.m23 = this->m21 * other.m13 + this->m22 * other.m23 + this->m23 * other.m33 + this->m24 * other.m43;
	retval.m24 = this->m21 * other.m14 + this->m22 * other.m24 + this->m23 * other.m34 + this->m24 * other.m44;

	retval.m31 = this->m31 * other.m11 + this->m32 * other.m21 + this->m33 * other.m31 + this->m34 * other.m41;
	retval.m32 = this->m31 * other.m12 + this->m32 * other.m22 + this->m33 * other.m32 + this->m34 * other.m42;
	retval.m33 = this->m31 * other.m13 + this->m32 * other.m23 + this->m33 * other.m33 + this->m34 * other.m43;
	retval.m34 = this->m31 * other.m14 + this->m32 * other.m24 + this->m33 * other.m34 + this->m34 * other.m44;

	retval.m41 = this->m41 * other.m11 + this->m42 * other.m21 + this->m43 * other.m31 + this->m44 * other.m41;
	retval.m42 = this->m41 * other.m12 + this->m42 * other.m22 + this->m43 * other.m32 + this->m44 * other.m42;
	retval.m43 = this->m41 * other.m13 + this->m42 * other.m23 + this->m43 * other.m33 + this->m44 * other.m43;
	retval.m44 = this->m41 * other.m14 + this->m42 * other.m24 + this->m43 * other.m34 + this->m44 * other.m44;

	return retval;
}
