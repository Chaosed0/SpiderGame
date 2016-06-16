#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <unordered_map>
#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "Material.h"

#define MAX_BONES_PER_VERTEX 4

/*! Vertex structure common to every mesh. */
struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
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
	/*! Vertex array object used to draw this model. */
	GLuint VAO;
	
	/* Vertex buffer object, containing Vertex structs. */
	GLuint VBO;

	/*! Vertex buffer object containing VertexBoneData structs. */
	GLuint VBO_bone;

	/*! Element buffer object. */
	GLuint EBO;

	/*! Total number of vertices in VBO and VBO_bone. */
	GLuint nVertices;

	/*! Total number of indices in EBO. */
	GLuint nIndices;

	/*! Material to apply to the mesh. */
	Material material;

	/*! Bone data of the mesh. */
	std::vector<BoneData> boneData;

	/*! Default constructor */
	Mesh();

	/*!
	 * \brief Initializes a mesh with no bone data.
	 */
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);

	/*!
	 * \brief Initializes a mesh with bone data.
	 * \param vertexBoneData Vertex weights. The boneIds member of each VertexBoneData struct points
	 *		to the indices of bones in boneData.
	 */
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures, std::vector<VertexBoneData> vertexBoneData, std::vector<BoneData> boneData);

	/*!
	 * \brief Gets the transforms of each bone in boneData given the position
	 * \param nodeTransforms The transforms of all of the nodes in the model.
	 * \return Matrices for each bone which transform from the bone's bind-pose space to
	 *		the bone's new space (given by nodeTransforms[bone.nodeId]).

	 *		The values in the returned vector directly correspond to the bones in this mesh's
	 *		boneData vector.
	 */
	std::vector<glm::mat4> getBoneTransforms(const std::vector<glm::mat4>& nodeTransforms) const;
};

/*! Animation channel corresponding to one node in the model.
	Note that the keys contain the absolute transform of the bone relative to its parent.
	They are not relative to the bind-pose transform of the bone. */
struct Channel
{
	/*! The node which this channel corresponds to. */
	unsigned int nodeId;

	/*! Position keyframes.
		The first element contains the keyframe time in seconds, and the second element
		contains the position at that time. */
	std::vector<std::pair<float, glm::vec3>> positionKeys;

	/*! Rotation keyframes. */
	std::vector<std::pair<float, glm::quat>> rotationKeys;

	/*! Scale keyframes. */
	std::vector<std::pair<float, glm::vec3>> scaleKeys;
};

/*! Data for a model's single animation. */
struct Animation
{
	/*! Total duration of the animation. */
	float duration;

	/*! Map of node IDs to the corresponding channel index in channels.
		Necessary because not all nodes are animated. */
	std::unordered_map<unsigned int, unsigned int> channelIdMap;

	/*! Vector containing all the channels of the animation. */
	std::vector<Channel> channels;
};

/*! Node in the model's hierarchy. This is strongly tied to an AnimationData struct.
	An "index" here refers to an index into AnimationData.nodes. */
struct ModelNode
{
	/*! Name of this node, purely for debugging purposes. */
	std::string name;

	/*! Transform of this node. */
	glm::mat4 transform;

	/*! True if the node is the root node. */
	bool isRoot;

	/*! Index of this node's parent. */
	unsigned int parent;

	/*! Indices of this node's children. */
	std::vector<unsigned int> children;
};

/*! Data for all the animations which a model contains. */
struct AnimationData
{
	/*! The animations themselves. */
	std::unordered_map<std::string, Animation> animations;

	/*! A map of node names to node IDs. (Is this necessary?) */
	std::unordered_map<std::string, unsigned int> nodeIdMap;

	/*! The node hierarchy. */
	std::vector<ModelNode> nodes;
};

/*! A model, containing multiple meshes and animation data. */
struct Model
{
	/*! Meshes which this model is made from. */
	std::vector<Mesh> meshes;

	/*! All the animations. */
	AnimationData animationData;

	/*! Model ID, used by the renderer.*/
	unsigned int id;

	Model() {}
	Model(std::vector<Mesh> meshes) : meshes(meshes) {}
	Model(std::vector<Mesh> meshes, AnimationData animationData) : meshes(meshes), animationData(animationData) {}

	/*!
	 * \brief Gets the transforms of each node at a certain time in an animation.
	 * \return Vector of matrices which transform from model space to each node's new space.
	 *		The matrices correspond directly to the nodes in animationData.nodes. 
	 */
	std::vector<glm::mat4> getNodeTransforms(const std::string& animation, float time) const;
};