
#include "Model.h"
#include "RenderUtil.h"

#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>

#include <glm/gtx/quaternion.hpp>

Mesh::Mesh()
{}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures, std::vector<VertexBoneData> vertexBoneData, std::vector<BoneData> boneData)
	: boneData(boneData)
{
	this->nVertices = vertices.size();
	this->nIndices = indices.size();

	assert(vertexBoneData.size() == 0 || vertexBoneData.size() == vertices.size());

	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);
	glCheckError();

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glCheckError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
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
		glGenBuffers(1, &this->VBO_bone);
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO_bone);
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

	for (unsigned int i = 0; i < textures.size(); i++) {
		MaterialProperty textureProperty(textures[i]);
		std::string key;
		if (textures[i].type == TextureType_diffuse) {
			key = "texture_diffuse";
		} else if (textures[i].type == TextureType_specular) {
			key = "texture_specular";
		} else if (textures[i].type == TextureType_cubemap) {
			key = "cubemap";
			// not all cubemaps are skyboxes, but they are for now!
			material.drawOrder = GL_LEQUAL;
		}
		material.setProperty(key, textureProperty);
	}

	MaterialProperty shininessProperty(32.0f);
	material.setProperty("shininess", shininessProperty);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
	: Mesh(vertices, indices, textures, std::vector<VertexBoneData>(), std::vector<BoneData>())
{ }

glm::vec3 interpolate(glm::vec3 a, glm::vec3 b, float lerp)
{
	return glm::mix(a, b, lerp);
}

glm::quat interpolate(glm::quat a, glm::quat b, float lerp)
{
	return glm::slerp(a, b, lerp);
}

template <class T>
T interpolateKeyframes(const std::vector<std::pair<float, T>>& keys, float time, unsigned& keyCache)
{
	unsigned keyIndex = keys.size()-1;
	time = std::fmin(std::fmax(time, keys[0].first), keys.back().first);

	for (unsigned int i = keyCache; i < keyCache + keys.size()-1; i++) {
		const auto& ka = keys[i%keys.size()];
		const auto& kb = (i+1 != keys.size() ? keys[(i+1)%keys.size()] : ka);
		if (ka.first <= time && time <= kb.first) {
			keyIndex = i%keys.size();
			break;
		}
	}

	keyCache = keyIndex;
	float t1 = keys[keyIndex].first;
	float t2 = (keyIndex + 1 < keys.size() ? keys[keyIndex+1].first : t1);
	const T& p1 = keys[keyIndex].second;
	const T& p2 = (keyIndex + 1 < keys.size() ? keys[keyIndex+1].second : p1);

	float lerp = (time - t1) / (t2 - t1);
	if (t2 == t1) {
		lerp = 1.0f;
	}

	return interpolate(p1, p2, lerp);
}

std::vector<glm::mat4> Mesh::getBoneTransforms(const std::vector<glm::mat4>& nodeTransforms) const
{
	std::vector<glm::mat4> boneTransforms;

	if (nodeTransforms.size() <= 0) {
		return boneTransforms;
	}

	// Assume 0 is the root node
	glm::mat4 globalInverse = glm::inverse(nodeTransforms[0]);
	boneTransforms.resize(this->boneData.size());
	for (unsigned int i = 0; i < this->boneData.size(); i++) {
		const BoneData& boneData = this->boneData[i];
		glm::mat4 nodeTransform = nodeTransforms[boneData.nodeId];
		boneTransforms[i] = globalInverse * nodeTransform * boneData.boneOffset;
	}
	
	return boneTransforms;
}

std::vector<glm::mat4> Model::getNodeTransforms(const std::string& animName, float time, AnimationContext& context) const
{
	std::vector<glm::mat4> nodeTransforms;

	auto iter = animationData.animations.find(animName);
	if (iter == animationData.animations.end()) {
		return nodeTransforms;
	}

	nodeTransforms.resize(animationData.nodes.size());
	const Animation& animation = iter->second;
	time += animation.startTime;

	for (unsigned i = 0; i < animationData.nodes.size(); i++) {
		const ModelNode& node = animationData.nodes[i];

		glm::mat4 parentTransform = glm::mat4();
		if (node.parent < animationData.nodes.size()) {
			 parentTransform = nodeTransforms[node.parent];
		}
		glm::mat4 nodeTransform;

		auto channelIdIter = animation.channelIdMap.find(i);
		if (channelIdIter == animation.channelIdMap.end()) {
			// Not an animated node
			nodeTransform = node.transform;
		} else {
			unsigned channelId = channelIdIter->second;
			const Channel& channel = animation.channels[channelId];
			ChannelContext& channelContext = context.channelContexts[channelId];

			glm::vec3 pos = interpolateKeyframes(channel.positionKeys, time, channelContext.positionKey);
			glm::quat rot = interpolateKeyframes(channel.rotationKeys, time, channelContext.rotationKey);
			glm::vec3 scale = interpolateKeyframes(channel.scaleKeys, time, channelContext.scaleKey);

			glm::mat4 posMatrix = glm::mat4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				pos.x, pos.y, pos.z, 1.0f
				);
			glm::mat4 rotMatrix(glm::toMat4(rot));
			glm::mat4 scaleMatrix = glm::mat4(
				scale.x, 0.0f, 0.0f, 0.0f,
				0.0f, scale.y, 0.0f, 0.0f,
				0.0f, 0.0f, scale.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);
			nodeTransform = posMatrix * rotMatrix * scaleMatrix;
		}
		glm::mat4 globalTransform = parentTransform * nodeTransform;
		nodeTransforms[i] = globalTransform;
	}

	return nodeTransforms;
}
