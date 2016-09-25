
#include "Renderer/ModelLoader.h"

#include "Renderer/TextureLoader.h"
#include "Renderer/Texture.h"
#include "Renderer/Mesh.h"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <cmath>

struct ModelLoader::Impl
{
	/*! The relative directory to the model we are currently loading. */
	std::string curDir;

	/*! Cache mapping IDs to models. The IDs can also be paths. */
	std::unordered_map<std::string, Model> modelIdCache;

	/*! Cache mapping IDs to textures. The IDs are usually paths. */
	std::unordered_map<std::string, TextureImpl> textureCache;

	/*! Used to load textures from imported models. */
	TextureLoader textureLoader;

	/*!
	 * \brief Processes an assimp model, starting from its root node.
	 */
	Model processRootNode(aiNode* node, const aiScene* scene);

	/*!
	 * \brief Processes a specific mesh of a model.
	 * \param nodeIdMap A map of node names to internal node IDs. Used when the bones are being loaded from the mesh.
	 */
	Model processMesh(aiMesh* mesh, const aiScene* scene, std::unordered_map<std::string, unsigned int> nodeIdMap);

	/*!
	 * \brief Processes and returns the bone data of a specific mesh.
	 * \param nodeIdMap Map of node names to internal node IDs.
	 * \param vertexBoneData Output parameter containing bone (node) IDs and the corresponding weights. The length
	 *		is equal to mesh->mNumVertices.
	 * \param boneData The data loaded from each of the bones.
	 */
	void loadBoneData(aiMesh* mesh, const aiScene* scene, std::unordered_map<std::string, unsigned int> nodeIdMap, std::vector<VertexBoneData>& vertexBoneData, std::vector<BoneData>& boneData);

	/*!
	 * \brief Loads textures from a material, but only of a specific type.
	 */
	std::vector<Texture> loadMaterialTextures(const std::string& relDir, aiMaterial* mat, aiTextureType type);

	// Utility functions
	glm::vec3 aiToGlm(aiVector3D vec3);
	glm::quat aiToGlm(aiQuaternion quat);
	glm::mat4 aiToGlm(aiMatrix4x4 mat4);
	glm::mat3 aiToGlm(aiMatrix3x3 mat3);
};

ModelLoader::ModelLoader()
	: impl(new Impl())
{
	impl->modelIdCache.emplace(std::make_pair("error", Model()));
}

ModelLoader::~ModelLoader() { }

Model ModelLoader::loadModelFromPath(const std::string& path)
{
	auto modelCacheIter = this->impl->modelIdCache.find(path);
	if (modelCacheIter != this->impl->modelIdCache.end()) {
		return modelCacheIter->second;
	} else {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			fprintf(stderr, "Assimp error while loading model: %s\n", importer.GetErrorString());
			return Model();
		}

		std::vector<Mesh> meshes;
		this->impl->curDir = path.substr(0, path.find_last_of('/')+1);
		Model model = this->impl->processRootNode(scene->mRootNode, scene);

		this->impl->modelIdCache.emplace(std::make_pair(path, model));
		return model;
	}
}

Model ModelLoader::Impl::processRootNode(aiNode* rootNode, const aiScene* scene)
{
	AnimationData animationData;

	std::vector<aiNode*> nodesWithMeshes;
	std::vector<aiNode*> processQueue;
	processQueue.push_back(rootNode);

	// First process the node hierarchy and pack it into our vector
	while (processQueue.size() > 0) {
		aiNode* ai_node = processQueue.back();
		processQueue.pop_back();
		std::string nodeName(ai_node->mName.data);

		// If this node has meshes, we'll want to save it for processing later
		if (ai_node->mNumMeshes > 0) {
			nodesWithMeshes.push_back(ai_node);
		}

		unsigned int nodeId = animationData.nodes.size();
		animationData.nodeIdMap[nodeName] = nodeId;

		ModelNode node;
		node.name = nodeName;
		node.transform = aiToGlm(ai_node->mTransformation);
		if (ai_node->mParent != NULL) {
			auto iter = animationData.nodeIdMap.find(ai_node->mParent->mName.data);
			assert(iter != animationData.nodeIdMap.end());
			// Assign parent
			node.parent = iter->second;
			// Assign self to parent's children
			animationData.nodes[iter->second].children.push_back(nodeId);
			node.isRoot = false;
		} else {
			node.isRoot = true;
		}

		animationData.nodes.push_back(node);

		for (unsigned int i = 0; i < ai_node->mNumChildren; i++) {
			processQueue.push_back(ai_node->mChildren[i]);
		}
	}

	// Let's assume each model only has a single mesh
	aiNode* meshNode = nodesWithMeshes[0];
	aiMesh* mesh = scene->mMeshes[meshNode->mMeshes[0]];
	Model model = this->processMesh(mesh, scene, animationData.nodeIdMap);

	// Process the animations
	for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation* ai_animation = scene->mAnimations[i];
		std::string animName(ai_animation->mName.data);
		Animation& animation = animationData.animations[animName];

		float minTime = FLT_MAX;
		float maxTime = FLT_MIN;
		for (unsigned int j = 0; j < ai_animation->mNumChannels; j++) {
			aiNodeAnim* ai_channel = ai_animation->mChannels[j];
			std::string nodeName = ai_channel->mNodeName.data;

			auto iter = animationData.nodeIdMap.find(nodeName);
			assert(iter != animationData.nodeIdMap.end());

			Channel channel;
			channel.nodeId = iter->second;
			channel.positionKeys.resize(ai_channel->mNumPositionKeys);
			channel.rotationKeys.resize(ai_channel->mNumRotationKeys);
			channel.scaleKeys.resize(ai_channel->mNumScalingKeys);

			for (unsigned int k = 0; k < ai_channel->mNumPositionKeys; k++) {
				aiVectorKey ai_posKey = ai_channel->mPositionKeys[k];
				channel.positionKeys[k].time = (float)(ai_posKey.mTime / ai_animation->mTicksPerSecond);
				channel.positionKeys[k].value = aiToGlm(ai_posKey.mValue);
				if (ai_channel->mNumPositionKeys > 1) {
					// If there's only one keyframe, ignore it for time calculation purposes
					minTime = std::fmin(minTime, (float)ai_channel->mPositionKeys[k].mTime);
					maxTime = std::fmax(maxTime, (float)ai_channel->mPositionKeys[k].mTime);
				}
			}
			for (unsigned int k = 0; k < ai_channel->mNumRotationKeys; k++) {
				aiQuatKey ai_rotKey = ai_channel->mRotationKeys[k];
				channel.rotationKeys[k].time = (float)(ai_rotKey.mTime / ai_animation->mTicksPerSecond);
				channel.rotationKeys[k].value = aiToGlm(ai_rotKey.mValue);
				if (ai_channel->mNumRotationKeys > 1) {
					// If there's only one keyframe, ignore it for time calculation purposes
					minTime = std::fmin(minTime, (float)ai_channel->mRotationKeys[k].mTime);
					maxTime = std::fmax(maxTime, (float)ai_channel->mRotationKeys[k].mTime);
				}
			}
			for (unsigned int k = 0; k < ai_channel->mNumScalingKeys; k++) {
				aiVectorKey ai_scaleKey = ai_channel->mScalingKeys[k];
				channel.scaleKeys[k].time = (float)(ai_scaleKey.mTime / ai_animation->mTicksPerSecond);
				channel.scaleKeys[k].value = aiToGlm(ai_scaleKey.mValue);
				if (ai_channel->mNumScalingKeys > 1) {
					// If there's only one keyframe, ignore it for time calculation purposes
					minTime = std::fmin(minTime, (float)ai_channel->mScalingKeys[k].mTime);
					maxTime = std::fmax(maxTime, (float)ai_channel->mScalingKeys[k].mTime);
				}
			}

			// Default to 30 ticks per second if unspecified
			float ticksPerSecond = (ai_animation->mTicksPerSecond != 0 ? (float)ai_animation->mTicksPerSecond : 1/30.0f);

			// In most cases, ignore ai_animation->mDuration - it seems to be inaccurate for
			// animations that don't start at time 0
			if (minTime != FLT_MIN) {
				animation.startTime = (float)(minTime / ticksPerSecond);
				animation.endTime = (float)(maxTime / ticksPerSecond);
			} else {
				animation.startTime = 0.0f;
				animation.endTime = (float)(ai_animation->mDuration / ticksPerSecond);
			}

			animation.channelIdMap[channel.nodeId] = animation.channels.size();
			animation.channels.push_back(channel);
		}
	}

	model.animationData = animationData;
	return model;
}

Model ModelLoader::Impl::processMesh(aiMesh* mesh, const aiScene* scene, std::unordered_map<std::string, unsigned int> nodeIdMap)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	for (unsigned i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		if (mesh->mTextureCoords[0]) {
			vertex.texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		} else {
			vertex.texCoords = glm::vec2(0.0f, 0.0f);
		}
		vertices.push_back(vertex);
	}

	for (unsigned i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = this->loadMaterialTextures(this->curDir, material, aiTextureType_DIFFUSE);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = this->loadMaterialTextures(this->curDir, material, aiTextureType_SPECULAR);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	std::vector<VertexBoneData> vertexBoneData;
	std::vector<BoneData> boneData;
	loadBoneData(mesh, scene, nodeIdMap, vertexBoneData, boneData);

	Material material;
	material.setTextures(textures);
	Mesh processedMesh(vertices, indices, vertexBoneData, boneData);
	Model model(processedMesh, material);
	return model;
}

void ModelLoader::Impl::loadBoneData(aiMesh* mesh, const aiScene* scene, std::unordered_map<std::string, unsigned int> nodeIdMap, std::vector<VertexBoneData>& vertexBoneData, std::vector<BoneData>& boneData)
{
	if (mesh->mNumBones > 0) {
		boneData.resize(mesh->mNumBones);
		vertexBoneData.resize(mesh->mNumVertices);
	}

	for (unsigned int i = 0; i < mesh->mNumBones; i++) {
		aiBone* bone = mesh->mBones[i];
		std::string boneName(bone->mName.data);

		auto iter = nodeIdMap.find(boneName);
		assert(iter != nodeIdMap.end());
		boneData[i].nodeId = iter->second;
		boneData[i].boneOffset = aiToGlm(bone->mOffsetMatrix);

		for (unsigned int j = 0; j < bone->mNumWeights; j++) {
			aiVertexWeight weight = bone->mWeights[j];
			vertexBoneData[weight.mVertexId].addWeight(i, weight.mWeight);
		}
	}
}

std::vector<Texture> ModelLoader::Impl::loadMaterialTextures(const std::string& relDir, aiMaterial* mat, aiTextureType type)
{
	std::vector<Texture> textures;
	for (unsigned i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string path(str.C_Str());
		path = relDir + path;

		Texture texture;
		auto cacheIter = this->textureCache.find(path);
		if (cacheIter == this->textureCache.end()) {
			TextureType newType = (type == aiTextureType_DIFFUSE ? TextureType_diffuse : TextureType_specular);
			texture = textureLoader.loadFromFile(newType, path);
			this->textureCache.emplace(std::make_pair(path, *texture.impl));
		} else {
			texture.impl = std::make_unique<TextureImpl>(cacheIter->second);
		}

		textures.emplace_back(texture);
	}
	return textures;
}

glm::vec3 ModelLoader::Impl::aiToGlm(aiVector3D vec3)
{
	return glm::vec3(vec3.x, vec3.y, vec3.z);
}

glm::quat ModelLoader::Impl::aiToGlm(aiQuaternion quat)
{
	// Assimp uses wxyz order, where glm uses xyzw
	return glm::quat(aiToGlm(quat.GetMatrix()));
}

glm::mat4 ModelLoader::Impl::aiToGlm(aiMatrix4x4 mat4)
{
	// Assimp matrices are column-major, where as glm's are row-major
	return glm::mat4(
			mat4.a1, mat4.b1, mat4.c1, mat4.d1,
			mat4.a2, mat4.b2, mat4.c2, mat4.d2,
			mat4.a3, mat4.b3, mat4.c3, mat4.d3,
			mat4.a4, mat4.b4, mat4.c4, mat4.d4 );
}

glm::mat3 ModelLoader::Impl::aiToGlm(aiMatrix3x3 mat3)
{
	return glm::mat3(
			mat3.a1, mat3.b1, mat3.c1,
			mat3.a2, mat3.b2, mat3.c2,
			mat3.a3, mat3.b3, mat3.c3 );
}