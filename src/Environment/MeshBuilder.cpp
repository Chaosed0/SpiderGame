
#include "MeshBuilder.h"

#include <algorithm>

MeshBuilder::MeshBuilder()
{
	ivArray = nullptr;
	meshShape = nullptr;
}

void MeshBuilder::addRoom(const Room& room, float height)
{
	for (unsigned i = 0; i < room.sides.size(); i++) {
		RoomSide side = room.sides[i];

		// These calculations brought to you by the right-hand rule
		int leftx, rightx, lefty, righty;
		if (side.normal.y > 0) {
			leftx = side.x0;
			rightx = side.x1;
		} else {
			leftx = side.x1;
			rightx = side.x0;
		}

		if (side.normal.x < 0) {
			lefty = side.y0;
			righty = side.y1;
		} else {
			lefty = side.y1;
			righty = side.y0;
		}

		this->addPlane(glm::vec3((float)leftx, height, (float)lefty),
			glm::vec3((float)rightx, height, (float)righty),
			glm::vec3((float)leftx, 0.0f, (float)lefty),
			glm::vec3((float)rightx, 0.0f, (float)righty));
	}

	for (unsigned i = 0; i < room.boxes.size(); i++) {
		RoomBox box = room.boxes[i];
		this->addPlane(glm::vec3((float)box.left, 0.0f, (float)box.bottom),
			glm::vec3((float)box.right, 0.0f, (float)box.bottom),
			glm::vec3((float)box.left, 0.0f, (float)box.top),
			glm::vec3((float)box.right, 0.0f, (float)box.top));
		/*this->addPlane(glm::vec3((float)box.left, height, (float)box.top),
			glm::vec3((float)box.right, height, (float)box.top),
			glm::vec3((float)box.left, height, (float)box.bottom),
			glm::vec3((float)box.right, height, (float)box.bottom));*/
	}
}

void MeshBuilder::addPlane(const glm::vec3& tlv, const glm::vec3& trv, const glm::vec3& blv, const glm::vec3& brv)
{
	glm::vec3 uvec = brv - blv;
	glm::vec3 vvec = tlv - blv;
	glm::vec3 normal = glm::normalize(glm::cross(uvec, vvec));

	// Top-left texture coord
	glm::vec2 tltc = glm::vec2(glm::dot(tlv, glm::normalize(uvec)), glm::dot(tlv, glm::normalize(vvec)));

	// Texture coord length
	glm::vec2 tcl = glm::vec2(glm::length(uvec), glm::length(vvec));

	unsigned tli = this->addVert(tlv, normal, tltc);
	unsigned tri = this->addVert(trv, normal, tltc + glm::vec2(tcl.x, 0.0f));
	unsigned bli = this->addVert(blv, normal, tltc + glm::vec2(0.0f, tcl.y));
	unsigned bri = this->addVert(brv, normal, tltc + tcl);

	this->indices.push_back(bli);
	this->indices.push_back(tri);
	this->indices.push_back(tli);
	this->indices.push_back(bli);
	this->indices.push_back(bri);
	this->indices.push_back(tri);
}

unsigned MeshBuilder::addVert(const glm::vec3& position, const glm::vec3& normal, const glm::vec2& texCoord)
{
	unsigned i = this->verts.size();
	Vertex v;
	v.position = position;
	v.normal = normal;
	v.texCoords = texCoord;
	this->verts.push_back(v);
	return i;
}

void MeshBuilder::construct()
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

Model MeshBuilder::getModel(std::vector<Texture>& textures) {
	std::vector<unsigned> modelIndices(indices.size());
	for (unsigned i = 0; i < indices.size(); i++) {
		modelIndices[i] = indices[i];
	}
	return Model(std::vector<Mesh>{ Mesh(verts, modelIndices, textures) });
}

btBvhTriangleMeshShape* MeshBuilder::getCollisionMesh()
{
	return meshShape;
}

MeshBuilder::~MeshBuilder()
{
	delete(ivArray);
	delete(meshShape);
}
