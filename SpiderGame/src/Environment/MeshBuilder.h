#pragma once

#include "btBulletDynamicsCommon.h"

#include <vector>

#include "Room.h"
#include "Renderer/Model.h"

class MeshBuilder
{
public:
	MeshBuilder();
	~MeshBuilder();

	void addRoom(const Room& room, float height);
	void addPlane(const glm::vec3& tlv, const glm::vec3& trv, const glm::vec3& blv, const glm::vec3& brv);
	void construct();
	btBvhTriangleMeshShape* getCollisionMesh();
	Model getModel(std::vector<Texture>& textures);
	void reset();
private:
	unsigned addVert(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord);

	std::vector<Vertex> verts;
	std::vector<btScalar> collisionVerts;
	std::vector<int> indices;
	btBvhTriangleMeshShape* meshShape;
	btTriangleIndexVertexArray* ivArray;
};