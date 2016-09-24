#pragma once

#include <glm/glm.hpp>

#include <vector>
#include <memory>

#include <GL/glew.h>

#include "Renderer/Texture.h"

#define MAX_BONES_PER_VERTEX 4

/*! Vertex structure common to every mesh. */
struct Vertex
{
	Vertex() : position(0.0f), normal(0.0f), texCoords(0.0f), tintColor(1.0f) { };
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
	glm::vec3 tintColor;
};

/*! Bone data for a single vertex. */
struct VertexBoneData {
	VertexBoneData() : boneWeights{0.0f}, boneIds{0} {}

	unsigned int boneIds[MAX_BONES_PER_VERTEX];
	float boneWeights[MAX_BONES_PER_VERTEX];

	void addWeight(unsigned int id, float weight) {
		for (int i = 0; i < MAX_BONES_PER_VERTEX; i++) {
			if (boneWeights[i] == 0) {
				boneWeights[i] = weight;
				boneIds[i] = id;
				return;
			}
		}

		fprintf(stderr, "WARNING: More than %d bones\n", MAX_BONES_PER_VERTEX);
	}
};

/*! Bone data for a single bone.
 *  Remember that bones are part of a mesh, and nodes are part of a model.
 */
struct BoneData {
	/*! Transforms a point from model space to local bone space. */
	glm::mat4 boneOffset;

	/*! The node in the model that this bone corresponds to. */
	unsigned int nodeId;
};

/*! A single mesh. */
struct Mesh
{
	struct Impl;
	std::unique_ptr<Impl> impl;

	/*! Default constructor */
	Mesh();
	~Mesh();
	Mesh(const Mesh& mesh);

	/*!
	 * \brief Initializes a mesh with no bone data.
	 */
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices);

	/*!
	 * \brief Initializes a mesh with bone data.
	 * \param vertexBoneData Vertex weights. The boneIds member of each VertexBoneData struct points
	 *		to the indices of bones in boneData.
	 */
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<VertexBoneData> vertexBoneData, std::vector<BoneData> boneData);

	/*!
	 * \brief Gets the transforms of each bone in boneData given the position
	 * \param nodeTransforms The transforms of all of the nodes in the model.
	 * \return Matrices for each bone which transform from the bone's bind-pose space to
	 *		the bone's new space (given by nodeTransforms[bone.nodeId]).

	 *		The values in the returned vector directly correspond to the bones in this mesh's
	 *		boneData vector.
	 */
	std::vector<glm::mat4> getBoneTransforms(const std::vector<glm::mat4>& nodeTransforms);
};
