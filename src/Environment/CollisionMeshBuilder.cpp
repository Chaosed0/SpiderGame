
#include "CollisionMeshBuilder.h"

CollisionMeshBuilder::CollisionMeshBuilder()
{
	ivArray = nullptr;
	meshShape = nullptr;
}

void CollisionMeshBuilder::addRoom(const Room& room, float height)
{
	for (unsigned i = 0; i < room.sides.size(); i++) {
		RoomSide side = room.sides[i];
		this->addPlane(btVector3((float)side.x0, height, (float)side.y0),
			btVector3((float)side.x1, height, (float)side.y1),
			btVector3((float)side.x0, 0.0f, (float)side.y0),
			btVector3((float)side.x1, 0.0f, (float)side.y1));
	}

	for (unsigned i = 0; i < room.boxes.size(); i++) {
		RoomBox box = room.boxes[i];
		this->addPlane(btVector3((float)box.left, 0.0f, (float)box.top),
			btVector3((float)box.right, 0.0f, (float)box.top),
			btVector3((float)box.left, 0.0f, (float)box.bottom),
			btVector3((float)box.right, 0.0f, (float)box.bottom));
	}
}

void CollisionMeshBuilder::addBox(const btVector3& v1, const btVector3& v2)
{
}

void CollisionMeshBuilder::addPlane(const btVector3& tlv, const btVector3& trv, const btVector3& blv, const btVector3& brv)
{
	unsigned tli = this->addVert(tlv);
	unsigned tri = this->addVert(trv);
	unsigned bli = this->addVert(blv);
	unsigned bri = this->addVert(brv);

	this->indices.push_back(tli);
	this->indices.push_back(tri);
	this->indices.push_back(bli);
	this->indices.push_back(tri);
	this->indices.push_back(bri);
	this->indices.push_back(bli);
}

unsigned CollisionMeshBuilder::addVert(const btVector3& vec3)
{
	unsigned i = this->verts.size() / 3;
	this->verts.push_back(vec3.x());
	this->verts.push_back(vec3.y());
	this->verts.push_back(vec3.z());
	return i;
}

void CollisionMeshBuilder::construct()
{
	if (ivArray != nullptr) {
		delete(ivArray);
		delete(meshShape);
	}

	ivArray = new btTriangleIndexVertexArray(this->indices.size() / 3, this->indices.data(), 3 * sizeof(unsigned), this->verts.size(), this->verts.data(), 3 * sizeof(float));
	meshShape = new btBvhTriangleMeshShape(ivArray, true);
}

btBvhTriangleMeshShape* CollisionMeshBuilder::getMesh()
{
	return meshShape;
}

CollisionMeshBuilder::~CollisionMeshBuilder()
{
	delete(ivArray);
	delete(meshShape);
}
