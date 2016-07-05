#pragma once

#include "btBulletDynamicsCommon.h"

#include <vector>

#include "Room.h"

class CollisionMeshBuilder
{
public:
	CollisionMeshBuilder();
	~CollisionMeshBuilder();

	void addRoom(const Room& room, float height);
	void addBox(const btVector3& v1, const btVector3& v2);
	void addPlane(const btVector3& tlv, const btVector3& trv, const btVector3& blv, const btVector3& brv);
	void construct();
	btBvhTriangleMeshShape* getMesh();
private:
	unsigned addVert(const btVector3& vec3);

	std::vector<btScalar> verts;
	std::vector<int> indices;
	btBvhTriangleMeshShape* meshShape;
	btTriangleIndexVertexArray* ivArray;
};