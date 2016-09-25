
#include "Renderer/Model.h"

#include "Renderer/RenderUtil.h"
#include "Renderer/Texture.h"
#include "Renderer/Shader.h"
#include "Renderer/Mesh.h"
#include "Math/Matrix.h"

#include <sstream>
#include <string>
#include <algorithm>
#include <cmath>

#include <glm/gtx/quaternion.hpp>

#include <GL/glew.h>

Model::Model()
{ }

Model::Model(const Mesh& mesh, const Material& material, const AnimationData& animationData)
	: mesh(mesh), material(material), animationData(animationData)
{ }

Model::Model(const Mesh& mesh, const Material& material)
	: mesh(mesh), material(material)
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

std::vector<glm::mat4> Model::getNodeTransforms(const std::string& animName, float time, AnimationContext& context)
{
	auto iter = animationData.animations.find(animName);
	if (iter == animationData.animations.end()) {
		return nodeTransforms;
	}

	if (nodeTransforms.size() < animationData.nodes.size()) {
		nodeTransforms.resize(animationData.nodes.size());
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
