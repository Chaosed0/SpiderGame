#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <unordered_map>
#include <vector>

#include "Renderer/Material.h"
#include "Renderer/Mesh.h"
#include "Renderer/TextureLoader.h"

#define MAX_BONES_PER_VERTEX 4

struct PositionKey
{
	float time;
	glm::vec3 value;
};

struct RotationKey
{
	float time;
	glm::quat value;
};

struct ScaleKey
{
	float time;
	glm::vec3 value;
};

/*! Animation channel corresponding to one node in the model.
	Note that the keys contain the absolute transform of the bone relative to its parent.
	They are not relative to the bind-pose transform of the bone. */
struct Channel
{
	/*! The node which this channel corresponds to. */
	unsigned int nodeId;

	/*! Position keyframes. */
	std::vector<PositionKey> positionKeys;

	/*! Rotation keyframes. */
	std::vector<RotationKey> rotationKeys;

	/*! Scale keyframes. */
	std::vector<ScaleKey> scaleKeys;
};

/*! Data for a model's single animation. */
struct Animation
{
	/*! Start time of the animation in seconds. */
	float startTime;

	/*! Ending time of the animation in seconds. */
	float endTime;

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

/*! Data specific to a channel for a specific model. */
struct ChannelContext
{
	/*! The last position key we used. */
	unsigned positionKey;

	/*! The last rotation key we used. */
	unsigned rotationKey;

	/*! The last scale key we used. */
	unsigned scaleKey;
};

/*! Data specific to the animation being played.
	Necessary since the same Model object can be shared across multiple entities. */
struct AnimationContext
{
	/*! Key caches. */
	std::unordered_map<unsigned, ChannelContext> channelContexts;
};

/*! A model, containing multiple meshes and animation data. */
struct Model
{
	/*! Mesh which comprises this model. */
	Mesh mesh;

	/*! Material to apply to the mesh. */
	Material material;

	/*! All the animations. */
	AnimationData animationData;

	/*! Cached transforms returned from getNodeTransforms. */
	std::vector<glm::mat4> nodeTransforms;

	Model();
	~Model();
	Model(const Model& model);
	Model(const Mesh& mesh, const Material& material, const AnimationData& animationData);

	/*!
	 * \brief Gets the transforms of each node at a certain time in an animation.
	 * \return Vector of matrices which transform from model space to each node's new space.
	 *		The matrices correspond directly to the nodes in animationData.nodes. 
	 */
	std::vector<glm::mat4> getNodeTransforms(const std::string& animation, float time, AnimationContext& context);
};