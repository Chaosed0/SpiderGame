
#include "Model.h"
#include "RenderUtil.h"

#include <sstream>
#include <string>

Mesh::Mesh()
{}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures, AnimationData animationData)
	: animationData(animationData)
{
	this->nVertices = vertices.size();
	this->nIndices = indices.size();

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
	glVertexAttribIPointer(3, MAX_BONES_PER_VERTEX, GL_UNSIGNED_INT, sizeof(VertexBoneData), (GLvoid*)offsetof(VertexBoneData, ids));
	glCheckError();

	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, MAX_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, sizeof(VertexBoneData), (GLvoid*)offsetof(VertexBoneData, weights));
	glCheckError();

	glBindVertexArray(0);
	glCheckError();

	for (unsigned int i = 0; i < textures.size(); i++) {
		MaterialProperty textureProperty;
		if (textures[i].type == TextureType_diffuse) {
			textureProperty.key = "texture_diffuse";
			textureProperty.type = MaterialPropertyType_texture;
			textureProperty.value.texture = textures[i];
		} else if (textures[i].type == TextureType_specular) {
			textureProperty.key = "texture_specular";
			textureProperty.type = MaterialPropertyType_texture;
			textureProperty.value.texture = textures[i];
		} else if (textures[i].type == TextureType_cubemap) {
			textureProperty.key = "cubemap";
			textureProperty.type = MaterialPropertyType_texture;
			textureProperty.value.texture = textures[i];
			// not all cubemaps are skyboxes, but they are for now!
			material.drawOrder = GL_LEQUAL;
		}
		material.setProperty(textureProperty);
	}

	MaterialProperty shininessProperty;
	shininessProperty.key = "shininess";
	shininessProperty.type = MaterialPropertyType_float;
	shininessProperty.value.flt = 32.0f;
	material.setProperty(shininessProperty);
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
	: Mesh(vertices, indices, textures, AnimationData())
{ }

glm::vec3 interpolatePosition(const Channel& channel, float time)
{
	unsigned int posKey = channel.positionKeys.size()-1;
	if (time < 0) {
		posKey = 0;
	} else {
		for (unsigned int i = 0; i < channel.positionKeys.size()-1; i++) {
			if (channel.positionKeys[i].first <= time && time <= channel.positionKeys[i+1].first) {
				posKey = i;
				break;
			}
		}
	}

	float t1 = channel.positionKeys[posKey].first;
	float t2 = t1;
	glm::vec3 p1 = channel.positionKeys[posKey].second;
	glm::vec3 p2 = p1;
	if (posKey + 1 < channel.positionKeys.size()) {
		t2 = channel.positionKeys[posKey+1].first;
		p2 = channel.positionKeys[posKey+1].second;
	}
	float lerp = (time - t1) / (t2 - t1);

	return glm::mix(p1, p2, lerp);
}

glm::quat interpolateRotation(const Channel& channel, float time)
{
	unsigned int rotKey = channel.rotationKeys.size()-1;
	if (time < 0) {
		rotKey = 0;
	} else {
		for (unsigned int i = 0; i < channel.rotationKeys.size()-1; i++) {
			if (channel.rotationKeys[i].first <= time && time <= channel.rotationKeys[i+1].first) {
				rotKey = i;
				break;
			}
		}
	}

	float t1 = channel.rotationKeys[rotKey].first;
	float t2 = t1;
	glm::quat p1 = channel.rotationKeys[rotKey].second;
	glm::quat p2 = p1;
	if (rotKey + 1 < channel.rotationKeys.size()) {
		t2 = channel.rotationKeys[rotKey+1].first;
		p2 = channel.rotationKeys[rotKey+1].second;
	}
	float lerp = (time - t1) / (t2 - t1);

	return glm::slerp(p1, p2, lerp);
}

glm::vec3 interpolateScale(const Channel& channel, float time)
{
	unsigned int scaleKey = channel.scaleKeys.size()-1;
	if (time < 0) {
		scaleKey = 0;
	} else {
		for (unsigned int i = 0; i < channel.scaleKeys.size()-1; i++) {
			if (channel.scaleKeys[i].first <= time && time <= channel.scaleKeys[i+1].first) {
				scaleKey = i;
				break;
			}
		}
	}

	float t1 = channel.scaleKeys[scaleKey].first;
	float t2 = t1;
	glm::vec3 p1 = channel.scaleKeys[scaleKey].second;
	glm::vec3 p2 = p1;
	if (scaleKey + 1 < channel.scaleKeys.size()) {
		t2 = channel.scaleKeys[scaleKey+1].first;
		p2 = channel.scaleKeys[scaleKey+1].second;
	}
	float lerp = (time - t1) / (t2 - t1);

	return glm::mix(p1, p2, lerp);
}

std::vector<Transform> Mesh::getBoneTransforms(const std::string& animName, float time)
{
	auto iter = animationData.animations.find(animName);
	if (iter == animationData.animations.end()) {
		return std::vector<Transform>{};
	}

	Animation& animation = iter->second;
	for (unsigned int i = 0; i < animation.channels.size(); i++) {
		Channel& channel = animation.channels[i];
		BoneInfo& boneInfo = animationData.boneInfo[channel.boneId];

		glm::vec3 pos = interpolatePosition(channel, time);
		glm::quat rot = interpolateRotation(channel, time);
		glm::vec3 scale = interpolateScale(channel, time);
	}
}
