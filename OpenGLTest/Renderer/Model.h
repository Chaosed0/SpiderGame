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
	Vertex() : boneWeights{0.0f}, boneIds{0} {}
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;

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

struct BoneInfo
{
	glm::mat4 BoneOffset;
	Transform transform;
};

struct Channel
{
	unsigned int boneId;
	std::vector<std::pair<float, glm::vec3>> positionKeys;
	std::vector<std::pair<float, glm::quat>> rotationKeys;
	std::vector<std::pair<float, glm::vec3>> scaleKeys;
};

struct Animation
{
	float duration;
	std::vector<Channel> channels;
};

struct AnimationData
{
	std::unordered_map<std::string, Animation> animations;
	std::unordered_map<std::string, unsigned int> boneIdMap;
	std::vector<BoneInfo> boneInfo;
};

struct Mesh
{
	GLuint VAO, VBO, EBO;
	GLuint nVertices, nIndices;
	Material material;
	AnimationData animationData;

	Mesh();
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures, AnimationData animationData);

	std::vector<Transform> getBoneTransforms(const std::string& animName, float time);
};

struct Model
{
	std::vector<Mesh> meshes;
	unsigned int id;
	Model() {}
	Model(std::vector<Mesh> meshes) : meshes(meshes) {}
};