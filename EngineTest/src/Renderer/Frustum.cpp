
#include "catch.hpp"
#include "Renderer/Frustum.h"

TEST_CASE ( "Cube frustum", "[frustum]" )
{
	FrustumConstructor constructor;
	constructor.nbl = glm::vec3(0.0f, 0.0f, 0.0f);
	constructor.nbr = glm::vec3(1.0f, 0.0f, 0.0f);
	constructor.ntl = glm::vec3(0.0f, 1.0f, 0.0f);
	constructor.ntr = glm::vec3(1.0f, 1.0f, 0.0f);
	constructor.fbl = glm::vec3(0.0f, 0.0f, -1.0f);
	constructor.fbr = glm::vec3(1.0f, 0.0f, -1.0f);
	constructor.ftl = glm::vec3(0.0f, 1.0f, -1.0f);
	constructor.ftr = glm::vec3(1.0f, 1.0f, -1.0f);
	Frustum frustum(constructor);

	REQUIRE ( frustum.isInside(glm::vec3(0.5f, 0.5f, -0.5f)) == true );
	REQUIRE ( frustum.isInside(glm::vec3(0.0f, 0.0f, 0.0f)) == true );
	REQUIRE ( frustum.isInside(glm::vec3(0.0f, 0.2f, 0.0f)) == true );
	REQUIRE ( frustum.isInside(glm::vec3(1.0f, 0.2f, -1.0f)) == true );
	REQUIRE ( frustum.isInside(glm::vec3(0.0f, -0.2f, 0.0f)) == false );
	REQUIRE ( frustum.isInside(glm::vec3(1.0f, -0.2f, 0.0f)) == false );
}