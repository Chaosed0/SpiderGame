#pragma once

#include <glm/glm.hpp>

#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <unordered_map>
#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "Material.h"
#include "Transform.h"

#define MAX_BONES_PER_VERTEX 4

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

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

struct BoneData {
	glm::mat4 boneOffset;
	unsigned int nodeId;
};

struct Mesh
{
	GLuint VAO, VBO, EBO, VBO_bone;
	GLuint nVertices, nIndices;
	Material material;
	std::vector<BoneData> boneData;

	Mesh();
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures, std::vector<VertexBoneData> vertexBoneData, std::vector<BoneData> boneData);

	std::vector<glm::mat4> getBoneTransforms(const std::vector<glm::mat4>& nodeTransforms) const;
};

struct Channel
{
	unsigned int nodeId;
	std::vector<std::pair<float, glm::vec3>> positionKeys;
	std::vector<std::pair<float, glm::quat>> rotationKeys;
	std::vector<std::pair<float, glm::vec3>> scaleKeys;
};

struct Animation
{
	float duration;
	std::unordered_map<unsigned int, unsigned int> channelIdMap;
	std::vector<Channel> channels;
};

struct ModelNode
{
	glm::mat4 transform;
	bool isRoot;
	unsigned int parent;
	std::vector<unsigned int> children;
};

struct AnimationData
{
	std::unordered_map<std::string, Animation> animations;
	std::unordered_map<std::string, unsigned int> nodeIdMap;
	std::vector<ModelNode> nodes;
};

struct Model
{
	std::vector<Mesh> meshes;
	AnimationData animationData;
	unsigned int id;

	Model() {}
	Model(std::vector<Mesh> meshes) : meshes(meshes) {}
	Model(std::vector<Mesh> meshes, AnimationData animationData) : meshes(meshes), animationData(animationData) {}

	std::vector<glm::mat4> getNodeTransforms(const std::string& animation, float time) const;
};