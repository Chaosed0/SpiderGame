
#include "Renderer/Mesh.h"
#include "Renderer/MeshImpl.h"

#include "Renderer/RenderUtil.h"
#include "Renderer/Texture.h"
#include "Renderer/Shader.h"
#include "Renderer/Material.h"
#include "Math/Matrix.h"

#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>

#include <glm/gtx/quaternion.hpp>

#include <GL/glew.h>

Mesh::Mesh()
	: impl(new Impl())
{ }

Mesh::~Mesh()
{ }

Mesh::Mesh(const Mesh& mesh)
	: impl(new Impl(*mesh.impl))
{ }

void Mesh::operator=(const Mesh& mesh)
{
	this->impl = std::unique_ptr<Impl>(new Impl(*mesh.impl));
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices, std::vector<VertexBoneData> vertexBoneData, std::vector<BoneData> boneData)
	: Mesh()
{
	impl->boneData = boneData;
	impl->boneTransforms.resize(impl->boneData.size());

	impl->nVertices = vertices.size();
	impl->nIndices = indices.size();

	assert(vertexBoneData.size() == 0 || vertexBoneData.size() == vertices.size());

	glGenVertexArrays(1, &impl->VAO);
	glGenBuffers(1, &impl->VBO);
	glGenBuffers(1, &impl->EBO);
	glCheckError();

	glBindVertexArray(impl->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, impl->VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glCheckError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, impl->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);
	glCheckError();

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glCheckError();

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glCheckError();

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));
	glCheckError();

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, tintColor));
	glCheckError();

	if (vertexBoneData.size() > 0) {
		glGenBuffers(1, &impl->VBO_bone);
		glBindBuffer(GL_ARRAY_BUFFER, impl->VBO_bone);
		glBufferData(GL_ARRAY_BUFFER, vertexBoneData.size() * sizeof(VertexBoneData), &vertexBoneData[0], GL_STATIC_DRAW);
		glCheckError();

		glEnableVertexAttribArray(4);
		glVertexAttribIPointer(4, MAX_BONES_PER_VERTEX, GL_UNSIGNED_INT, sizeof(VertexBoneData), (GLvoid*)offsetof(VertexBoneData, boneIds));
		glCheckError();

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, MAX_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (GLvoid*)offsetof(VertexBoneData, boneWeights));
		glCheckError();
	}

	glBindVertexArray(0);
	glCheckError();
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned> indices)
	: Mesh(vertices, indices, std::vector<VertexBoneData>(), std::vector<BoneData>())
{ }

std::vector<glm::mat4> Mesh::getBoneTransforms(const std::vector<glm::mat4>& nodeTransforms)
{
	if (nodeTransforms.size() <= 0) {
		return impl->boneTransforms;
	}

	// Assume 0 is the root node
	mat4 globalInverse = glm::inverse(nodeTransforms[0]);
	for (unsigned int i = 0; i < impl->boneData.size(); i++) {
		const BoneData& boneData = impl->boneData[i];
		mat4 nodeTransform = nodeTransforms[boneData.nodeId];
		mat4 boneOffset = boneData.boneOffset;
		mat4 boneTransform = globalInverse * nodeTransform * boneOffset;
		impl->boneTransforms[i] = boneTransform.toGlm();
	}
	
	return impl->boneTransforms;
}
