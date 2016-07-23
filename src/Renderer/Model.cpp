
#include "Model.h"
#include "RenderUtil.h"

#include "Math/Matrix.h"

#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>

#include <glm/gtx/quaternion.hpp>

Mesh::Mesh()
{}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures, std::vector<VertexBoneData> vertexBoneData, std::vector<BoneData> boneData)
	: boneData(boneData), boneTransforms(boneData.size())
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

template <class ValType, class KeyType>
ValType interpolateKeyframes(const std::vector<KeyType>& keys, float time, unsigned& keyCache)
{
	unsigned keysSize = keys.size();
	unsigned keyIndex = keysSize-1;

	if (time <= keys[0].time) {
		keyIndex = 0;
	} else if (time >= keys[keysSize-1].time) {
		keyIndex = keysSize-1;
	} else {
		for (unsigned int i = keyCache; i < keyCache + keysSize-1; i++) {
			const KeyType& ka = keys[i%keysSize];
			const KeyType& kb = (i+1 != keysSize ? keys[(i+1)%keysSize] : ka);
			if (ka.time <= time && time <= kb.time) {
				keyIndex = i%keysSize;
				break;
			}
		}
	}

	keyCache = keyIndex;
	float t1 = keys[keyIndex].time;
	float t2 = (keyIndex + 1 < keysSize ? keys[keyIndex+1].time : t1);
	const ValType& p1 = keys[keyIndex].value;
	const ValType& p2 = (keyIndex + 1 < keysSize ? keys[keyIndex+1].value : p1);

	float lerp = (time - t1) / (t2 - t1);
	if (t2 == t1) {
		lerp = 1.0f;
	}

	return interpolate(p1, p2, lerp);
}

std::vector<glm::mat4> Mesh::getBoneTransforms(const std::vector<glm::mat4>& nodeTransforms)
{
	if (nodeTransforms.size() <= 0) {
		return boneTransforms;
	}

	// Assume 0 is the root node
	mat4 globalInverse = glm::inverse(nodeTransforms[0]);
	for (unsigned int i = 0; i < this->boneData.size(); i++) {
		const BoneData& boneData = this->boneData[i];
		mat4 nodeTransform = nodeTransforms[boneData.nodeId];
		mat4 boneOffset = boneData.boneOffset;
		mat4 boneTransform = globalInverse * nodeTransform * boneOffset;
		boneTransforms[i] = boneTransform.toGlm();
	}
	
	return boneTransforms;
}

std::vector<glm::mat4> Model::getNodeTransforms(const std::string& animName, float time, AnimationContext& context)
{
	auto iter = animationData.animations.find(animName);
	if (iter == animationData.animations.end()) {
		return nodeTransforms;
	}

	const Animation& animation = iter->second;
	time += animation.startTime;

	for (unsigned i = 0; i < animationData.nodes.size(); i++) {
		const ModelNode& node = animationData.nodes[i];

		mat4 parentTransform = mat4();
		if (node.parent < animationData.nodes.size()) {
			 parentTransform = nodeTransforms[node.parent];
		}

		mat4 nodeTransform;

		auto channelIdIter = animation.channelIdMap.find(i);
		if (channelIdIter == animation.channelIdMap.end()) {
			// Not an animated node
			nodeTransform = node.transform;
		} else {
			unsigned channelId = channelIdIter->second;
			const Channel& channel = animation.channels[channelId];
			ChannelContext& channelContext = context.channelContexts[channelId];

			glm::vec3 pos = interpolateKeyframes<glm::vec3, PositionKey>(channel.positionKeys, time, channelContext.positionKey);
			glm::quat rot = interpolateKeyframes<glm::quat, RotationKey>(channel.rotationKeys, time, channelContext.rotationKey);
			glm::vec3 scale = interpolateKeyframes<glm::vec3, ScaleKey>(channel.scaleKeys, time, channelContext.scaleKey);

			mat4 posMatrix = mat4(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				pos.x, pos.y, pos.z, 1.0f
				);
			mat4 rotMatrix(glm::toMat4(rot));
			mat4 scaleMatrix = mat4(
				scale.x, 0.0f, 0.0f, 0.0f,
				0.0f, scale.y, 0.0f, 0.0f,
				0.0f, 0.0f, scale.z, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
				);
			nodeTransform = posMatrix * rotMatrix * scaleMatrix;
		}
		mat4 globalTransform = parentTransform * nodeTransform;
		nodeTransforms[i] = globalTransform.toGlm();
	}

	return nodeTransforms;
}
