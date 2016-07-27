
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
		this->addPlane(glm::vec3((float)side.x0, height, (float)side.y0),
			glm::vec3((float)side.x1, height, (float)side.y1),
			glm::vec3((float)side.x0, 0.0f, (float)side.y0),
			glm::vec3((float)side.x1, 0.0f, (float)side.y1));
	}

	for (unsigned i = 0; i < room.boxes.size(); i++) {
		RoomBox box = room.boxes[i];
		this->addPlane(glm::vec3((float)box.left, 0.0f, (float)box.top),
			glm::vec3((float)box.right, 0.0f, (float)box.top),
			glm::vec3((float)box.left, 0.0f, (float)box.bottom),
			glm::vec3((float)box.right, 0.0f, (float)box.bottom));
	}
}

void CollisionMeshBuilder::addPlane(const glm::vec3& tlv, const glm::vec3& trv, const glm::vec3& blv, const glm::vec3& brv)
{
	glm::vec3 uvec = brv - blv;
	glm::vec3 vvec = tlv - blv;
	glm::vec3 normal = glm::normalize(glm::cross(uvec, vvec));
	glm::vec2 tltc = glm::vec2(glm::dot(tlv, uvec), glm::dot(tlv, vvec));
	glm::vec2 tcl = glm::vec2(glm::length(uvec), glm::length(vvec));
	unsigned tli = this->addVert(tlv, normal, tltc);
	unsigned tri = this->addVert(trv, normal, tltc + glm::vec2(tcl.x, 0.0f));
	unsigned bli = this->addVert(blv, normal, tltc + glm::vec2(0.0f, tcl.y));
	unsigned bri = this->addVert(brv, normal, tltc + tcl);

	this->indices.push_back(tli);
	this->indices.push_back(tri);
	this->indices.push_back(bli);
	this->indices.push_back(tri);
	this->indices.push_back(bri);
	this->indices.push_back(bli);
}

unsigned CollisionMeshBuilder::addVert(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord)
{
	unsigned i = this->verts.size();
	Vertex v;
	v.position = position;
	v.normal = normal;
	v.texCoords = texCoord;
	this->verts.push_back(v);
	return i;
}

void CollisionMeshBuilder::construct()
{
	if (ivArray != nullptr) {
		delete(ivArray);
		delete(meshShape);
	}

	for (unsigned i = 0; i < verts.size(); i++) {
		this->collisionVerts.push_back(verts[i].position.x);
		this->collisionVerts.push_back(verts[i].position.y);
		this->collisionVerts.push_back(verts[i].position.z);
	}

	ivArray = new btTriangleIndexVertexArray(this->indices.size() / 3, this->indices.data(), 3 * sizeof(unsigned), this->collisionVerts.size(), this->collisionVerts.data(), 3 * sizeof(float));
	meshShape = new btBvhTriangleMeshShape(ivArray, true);
}

Model CollisionMeshBuilder::getModel(std::vector<Texture>& textures) {
	std::vector<unsigned> modelIndices(indices.size());
	for (unsigned i = 0; i < indices.size(); i++) {
		modelIndices[i] = indices[i];
	}
	return Model(std::vector<Mesh>{ Mesh(verts, modelIndices, textures) });
}

btBvhTriangleMeshShape* CollisionMeshBuilder::getCollisionMesh()
{
	return meshShape;
}

CollisionMeshBuilder::~CollisionMeshBuilder()
{
	delete(ivArray);
	delete(meshShape);
}
