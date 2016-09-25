#pragma once

#include <glm/glm.hpp>

struct FrustumConstructor
{
	glm::vec3 ntl, ntr, nbl, nbr;
	glm::vec3 ftl, ftr, fbl, fbr;
};

class Frustum
{
public:
	Frustum();
	Frustum(const FrustumConstructor& constructor);
	bool isInside(const glm::vec3& point);
private:
	struct Plane {
		Plane();
		Plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2);
		bool onPositiveSide(const glm::vec3& point);
		glm::vec3 point, normal;
	};

	Plane nearPlane;
	Plane farPlane;
	Plane topPlane;
	Plane leftPlane;
	Plane rightPlane;
	Plane bottomPlane;
};