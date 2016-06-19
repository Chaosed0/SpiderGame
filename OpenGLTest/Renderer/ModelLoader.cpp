
#include "ModelLoader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

ModelLoader::ModelLoader()
{
	nextId = 1;
	modelIdCache["error"] = Model(std::vector<Mesh>());
}

Model ModelLoader::loadModelFromPath(const std::string& path)
{
	auto modelCacheIter = this->modelIdCache.find(path);
	if (modelCacheIter != this->modelIdCache.end()) {
		return modelCacheIter->second;
	} else {
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			fprintf(stderr, "Assimp error while loading model: %s\n", importer.GetErrorString());
			return Model(std::vector<Mesh>());
		}

		std::vector<Mesh> meshes;
		this->curDir = path.substr(0, path.find_last_of('/')+1);
		Model model = this->processRootNode(scene->mRootNode, scene);

		this->modelIdCache[path] = model;
		model.id = nextId;
		nextId++;
		return model;
	}
}

Model ModelLoader::loadModelById(const std::string& id)
{
	auto modelCacheIter = this->modelIdCache.find(id);
	if (modelCacheIter != this->modelIdCache.end()) {
		return modelCacheIter->second;
	} else {
		return this->modelIdCache["error"];
	}
}

void ModelLoader::assignModelToId(const std::string& id, Model model)
{
	model.id = nextId++;
	this->modelIdCache[id] = model;
}

Model ModelLoader::processRootNode(aiNode* rootNode, const aiScene* scene)
{
	std::vector<Mesh> meshes;
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

	// Find meshes, passing it the nodes (to allow mapping between bones/nodes)
	for (unsigned int i = 0; i < nodesWithMeshes.size(); i++) {
		aiNode* node = nodesWithMeshes[i];
		for (unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(this->processMesh(mesh, scene, animationData.nodeIdMap));
		}
	}

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
				channel.positionKeys[k].first = (float)(ai_posKey.mTime / ai_animation->mTicksPerSecond);
				channel.positionKeys[k].second = aiToGlm(ai_posKey.mValue);
				if (ai_channel->mNumPositionKeys > 1) {
					// If there's only one keyframe, ignore it for time calculation purposes
					minTime = min(minTime, (float)ai_channel->mPositionKeys[k].mTime);
					maxTime = max(maxTime, (float)ai_channel->mPositionKeys[k].mTime);
				}
			}
			for (unsigned int k = 0; k < ai_channel->mNumRotationKeys; k++) {
				aiQuatKey ai_rotKey = ai_channel->mRotationKeys[k];
				channel.rotationKeys[k].first = (float)(ai_rotKey.mTime / ai_animation->mTicksPerSecond);
				channel.rotationKeys[k].second = aiToGlm(ai_rotKey.mValue);
				if (ai_channel->mNumRotationKeys > 1) {
					// If there's only one keyframe, ignore it for time calculation purposes
					minTime = min(minTime, (float)ai_channel->mRotationKeys[k].mTime);
					maxTime = max(maxTime, (float)ai_channel->mRotationKeys[k].mTime);
				}
			}
			for (unsigned int k = 0; k < ai_channel->mNumScalingKeys; k++) {
				aiVectorKey ai_scaleKey = ai_channel->mScalingKeys[k];
				channel.scaleKeys[k].first = (float)(ai_scaleKey.mTime / ai_animation->mTicksPerSecond);
				channel.scaleKeys[k].second = aiToGlm(ai_scaleKey.mValue);
				if (ai_channel->mNumScalingKeys > 1) {
					// If there's only one keyframe, ignore it for time calculation purposes
					minTime = min(minTime, (float)ai_channel->mScalingKeys[k].mTime);
					maxTime = max(maxTime, (float)ai_channel->mScalingKeys[k].mTime);
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

	return Model(meshes, animationData);
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene, std::unordered_map<std::string, unsigned int> nodeIdMap)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;

	for (GLuint i = 0; i < mesh->mNumVertices; i++)
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

	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++) {
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

	return Mesh(vertices, indices, textures, vertexBoneData, boneData);
}

void ModelLoader::loadBoneData(aiMesh* mesh, const aiScene* scene, std::unordered_map<std::string, unsigned int> nodeIdMap, std::vector<VertexBoneData>& vertexBoneData, std::vector<BoneData>& boneData)
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

std::vector<Texture> ModelLoader::loadMaterialTextures(const std::string& relDir, aiMaterial* mat, aiTextureType type)
{
	std::vector<Texture> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string path(str.C_Str());
		path = relDir + path;

		Texture texture;
		auto cacheIter = this->textureCache.find(path);
		if (cacheIter == this->textureCache.end()) {
			if (texture.loadFromFile(path)) {
				texture.type = (type == aiTextureType_DIFFUSE ? TextureType_diffuse : TextureType_specular);
				this->textureCache[path] = texture;
			}
		} else {
			texture = cacheIter->second;
		}

		textures.push_back(texture);
	}
	return textures;
}

glm::vec3 ModelLoader::aiToGlm(aiVector3D vec3)
{
	return glm::vec3(vec3.x, vec3.y, vec3.z);
}

glm::quat ModelLoader::aiToGlm(aiQuaternion quat)
{
	// Assimp uses wxyz order, where glm uses xyzw
	return glm::quat(aiToGlm(quat.GetMatrix()));
}

glm::mat4 ModelLoader::aiToGlm(aiMatrix4x4 mat4)
{
	// Assimp matrices are column-major, where as glm's are row-major
	return glm::mat4(
			mat4.a1, mat4.b1, mat4.c1, mat4.d1,
			mat4.a2, mat4.b2, mat4.c2, mat4.d2,
			mat4.a3, mat4.b3, mat4.c3, mat4.d3,
			mat4.a4, mat4.b4, mat4.c4, mat4.d4 );
}

glm::mat3 ModelLoader::aiToGlm(aiMatrix3x3 mat3)
{
	return glm::mat3(
			mat3.a1, mat3.b1, mat3.c1,
			mat3.a2, mat3.b2, mat3.c2,
			mat3.a3, mat3.b3, mat3.c3 );
}