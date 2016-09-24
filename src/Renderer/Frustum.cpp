
#include "Frustum.h"

Frustum::Frustum()
{ }

Frustum::Frustum(const FrustumConstructor& constructor) :
	nearPlane(constructor.ntl, constructor.ntr, constructor.nbl),
	farPlane(constructor.ftl, constructor.fbl, constructor.ftr),
	topPlane(constructor.ntl, constructor.ftl, constructor.ntr),
	bottomPlane(constructor.nbl, constructor.nbr, constructor.fbl),
	leftPlane(constructor.ntl, constructor.nbl, constructor.ftl),
	rightPlane(constructor.ntr, constructor.ftr, constructor.nbr)
{ }

bool Frustum::isInside(const glm::vec3& point)
{
	return nearPlane.onPositiveSide(point) &&
		farPlane.onPositiveSide(point) &&
		topPlane.onPositiveSide(point) &&
		bottomPlane.onPositiveSide(point) &&
		leftPlane.onPositiveSide(point) &&
		rightPlane.onPositiveSide(point);
}

Frustum::Plane::Plane() { }

Frustum::Plane::Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2)
{
	point = p0;
	normal = glm::cross(p1 - p0, p2 - p0);
}

bool Frustum::Plane::onPositiveSide(const glm::vec3& point)
{
	float dotResult = glm::dot(point, this->normal) - glm::dot(this->point, this->normal);
	return dotResult >= 0.0f;
}
