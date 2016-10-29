
#include "Renderer/Box.h"

#include "Renderer/Mesh.h"
#include "Renderer/TextureLoader.h"

#include <GL/glew.h>

static const GLfloat vertex_data[] = {
	// Positions           // Normals           // Texture Coords
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,

	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
	0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
	0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
	0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
	0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f
};

Model getBox(const std::vector<Texture>& textures, glm::vec3 scale, glm::vec3 offset)
{
	std::vector<Vertex> vertices(36);
	for (unsigned i = 0; i < vertices.size(); i++) {
		vertices[i].position = glm::vec3(vertex_data[i*8] * scale.x, vertex_data[i*8+1] * scale.y, vertex_data[i*8+2] * scale.z) + offset;
		vertices[i].normal = glm::vec3(vertex_data[i*8+3], vertex_data[i*8+4], vertex_data[i*8+5]);
		vertices[i].texCoords = glm::vec2(vertex_data[i*8+6], vertex_data[i*8+7]);
	}

	std::vector<GLuint> indexes(36);
	for (unsigned i = 0; i < indexes.size(); i++) {
		indexes[i] = i;
	}

	Mesh mesh(vertices, indexes);
	Material material;
	material.setTextures(textures);
	return Model(mesh, material);
}

GLfloat skybox_verts[] = {
	// Positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	1.0f,  1.0f, -1.0f,
	1.0f,  1.0f,  1.0f,
	1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f, -1.0f,
	1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	1.0f, -1.0f,  1.0f
};

Model getSkybox(const std::vector<std::string>& skyboxTextures)
{ 
	std::vector<Vertex> vertices(36);
	for (unsigned int i = 0; i < vertices.size(); i++) {
		vertices[i].position = glm::vec3(skybox_verts[i*3], skybox_verts[i*3+1], skybox_verts[i*3+2]);
	}

	std::vector<GLuint> indexes(36);
	for (unsigned int i = 0; i < indexes.size(); i++) {
		indexes[i] = i;
	}

	std::vector<Texture> textures;
	TextureLoader textureLoader;
	Texture texture = textureLoader.loadCubemap(skyboxTextures);
	textures.push_back(texture);

	Mesh mesh(vertices, indexes);
	Material material;
	material.setTextures(textures);
	return Model(mesh, material);
}

Model getPlane(const std::vector<Texture>& textures, glm::vec3 ubasis, glm::vec3 vbasis, glm::vec2 dimensions, glm::vec2 textureOffset, glm::vec2 textureScale)
{
	float ltc = textureOffset.x;
	float rtc = textureOffset.x + textureScale.x;
	float btc = textureOffset.y;
	float ttc = textureOffset.y + textureScale.y;

	std::vector<Vertex> vertices(4);
	vertices[0].position = (-ubasis * dimensions.x - vbasis * dimensions.y) * 0.5f;
	vertices[0].texCoords = glm::vec2(ltc, ttc);
	vertices[1].position = (ubasis * dimensions.x - vbasis * dimensions.y) * 0.5f;
	vertices[1].texCoords = glm::vec2(rtc, ttc);
	vertices[2].position = (-ubasis * dimensions.x + vbasis * dimensions.y) * 0.5f;
	vertices[2].texCoords = glm::vec2(ltc, btc);
	vertices[3].position = (ubasis * dimensions.x + vbasis * dimensions.y) * 0.5f;
	vertices[3].texCoords = glm::vec2(rtc, btc);
	vertices[0].normal = vertices[1].normal = vertices[2].normal = vertices[3].normal = glm::cross(ubasis, vbasis);

	std::vector<unsigned> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(1);
	indices.push_back(3);
	indices.push_back(2);

	Mesh mesh(vertices, indices);
	Material material;
	material.setTextures(textures);
	return Model(mesh, material);
}

Model getDebugBoxMesh(const glm::vec3& halfExtents)
{
	static unsigned topRightFronti = 0;
	static unsigned topLeftFronti = 1;
	static unsigned topRightBacki = 2;
	static unsigned topLeftBacki = 3;
	static unsigned botRightFronti = 4;
	static unsigned botLeftFronti = 5;
	static unsigned botRightBacki = 6;
	static unsigned botLeftBacki = 7;

	// We're not expecting this to be lit nor textured, so ignore normal and texCoord
	std::vector<Vertex> vertices(8);
	vertices[topRightFronti].position = glm::vec3(halfExtents.x, halfExtents.y, halfExtents.z);
	vertices[topLeftFronti].position = glm::vec3(-halfExtents.x, halfExtents.y, halfExtents.z);
	vertices[topRightBacki].position = glm::vec3(halfExtents.x, halfExtents.y, -halfExtents.z);
	vertices[topLeftBacki].position = glm::vec3(-halfExtents.x, halfExtents.y, -halfExtents.z);
	vertices[botRightFronti].position = glm::vec3(halfExtents.x, -halfExtents.y, halfExtents.z);
	vertices[botLeftFronti].position = glm::vec3(-halfExtents.x, -halfExtents.y, halfExtents.z);
	vertices[botRightBacki].position = glm::vec3(halfExtents.x, -halfExtents.y, -halfExtents.z);
	vertices[botLeftBacki].position = glm::vec3(-halfExtents.x, -halfExtents.y, -halfExtents.z);

	std::vector<unsigned> indices;
	// Top face
	indices.push_back(topLeftFronti);
	indices.push_back(topRightFronti);
	indices.push_back(topRightFronti);
	indices.push_back(topRightBacki);
	indices.push_back(topRightBacki);
	indices.push_back(topLeftBacki);
	indices.push_back(topLeftBacki);
	indices.push_back(topLeftFronti);
	// Bottom face
	indices.push_back(botLeftFronti);
	indices.push_back(botRightFronti);
	indices.push_back(botRightFronti);
	indices.push_back(botRightBacki);
	indices.push_back(botRightBacki);
	indices.push_back(botLeftBacki);
	indices.push_back(botLeftBacki);
	indices.push_back(botLeftFronti);
	// Connecting bars
	indices.push_back(topLeftFronti);
	indices.push_back(botLeftFronti);
	indices.push_back(topRightFronti);
	indices.push_back(botRightFronti);
	indices.push_back(topLeftBacki);
	indices.push_back(botLeftBacki);
	indices.push_back(topRightBacki);
	indices.push_back(botRightBacki);

	Mesh mesh(vertices, indices);
	Material material;
	material.drawType = MaterialDrawType_Lines;
	return Model(mesh, material);
}