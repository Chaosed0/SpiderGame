
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
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
			fprintf(stderr, "Assimp error while loading model: %s\n", importer.GetErrorString());
			return Model(std::vector<Mesh>());
		}

		std::vector<Mesh> meshes;
		this->curDir = path.substr(0, path.find_last_of('/')+1);
		this->processNode(scene->mRootNode, scene, meshes);

		Model model(meshes);
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

void ModelLoader::assignModelToId(const std::string& id, std::vector<Mesh> meshes)
{
	Model model(meshes);
	model.id = nextId++;
	this->modelIdCache[id] = model;
}

void ModelLoader::processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes)
{
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(this->processMesh(mesh, scene));
	}

	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		this->processNode(node->mChildren[i], scene, meshes);
	}
}

Mesh ModelLoader::processMesh(aiMesh* mesh, const aiScene* scene)
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

	AnimationData animationData = loadBoneData(mesh, scene);

	return Mesh(vertices, indices, textures, animationData);
}

AnimationData ModelLoader::loadBoneData(aiMesh* mesh, const aiScene* scene)
{
	AnimationData animationData;

	animationData.vertexBoneData.resize(mesh->mNumVertices);
	animationData.boneInfo.resize(mesh->mNumBones);

	for (unsigned int i = 0; i < mesh->mNumBones; i++) {
		aiBone* bone = mesh->mBones[i];
		std::string boneName(bone->mName.data);
		animationData.boneIdMap[boneName] = i;

		aiMatrix4x4 offset = bone->mOffsetMatrix;
		animationData.boneInfo[i].BoneOffset = glm::mat4(
			offset.a1, offset.a2, offset.a3, offset.a4,
			offset.b1, offset.b2, offset.b3, offset.b4,
			offset.c1, offset.c2, offset.c3, offset.c4,
			offset.d1, offset.d2, offset.d3, offset.d4 );

		for (unsigned int j = 0; j < bone->mNumWeights; j++) {
			animationData.vertexBoneData[bone->mWeights[j].mVertexId].addWeight(i, bone->mWeights[j].mWeight);
		}
	}

	std::vector<aiNode*> traverseQueue;
	traverseQueue.push_back(scene->mRootNode);
	while (traverseQueue.size() > 0) {
		aiNode* node = traverseQueue.back();
		traverseQueue.pop_back();

		for (int i = 0; i < node->mNumChildren; i++) {
			traverseQueue.push_back(node->mChildren[i]);
		}

		std::string boneName(node->mName.data);
		auto iter = animationData.boneIdMap.find(boneName);
		if (iter == animationData.boneIdMap.end()) {
			// Not a node we care about
			continue;
		}

		BoneInfo boneInfo = animationData.boneInfo[iter->second];
		boneInfo.transform = aiToGame(node->mTransformation);

		if (node->mParent == NULL) {
			continue;
		}

		std::string parentBoneName(node->mParent->mName.data);
		auto parentIter = animationData.boneIdMap.find(parentBoneName);
		if (parentIter == animationData.boneIdMap.end()) {
			// Parent not found, but we were found? This could be bad...
			continue;
		}

		BoneInfo parentBoneInfo = animationData.boneInfo[parentIter->second];
		parentBoneInfo.transform.addChild(&boneInfo.transform);
	}

	std::unordered_map<std::string, Animation>& animations = animationData.animations;
	for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
		aiAnimation* ai_animation = scene->mAnimations[i];
		std::string animName(ai_animation->mName.data);
		Animation& animation = animations[animName];

		animation.duration = ai_animation->mDuration / ai_animation->mTicksPerSecond;
		
		for (unsigned int j = 0; j < ai_animation->mNumChannels; j++) {
			aiNodeAnim* ai_channel = ai_animation->mChannels[j];
			std::string boneName = ai_channel->mNodeName.data;
			auto iter = animationData.boneIdMap.find(boneName);
			if (iter == animationData.boneIdMap.end()) {
				// We don't care about this node - it doesn't have any weight
				continue;
			}

			animation.channels.emplace_back();
			Channel& channel = animation.channels.back();
			channel.boneId = animationData.boneIdMap[boneName];
			
			channel.positionKeys.resize(ai_channel->mNumPositionKeys);
			channel.rotationKeys.resize(ai_channel->mNumRotationKeys);
			channel.scaleKeys.resize(ai_channel->mNumScalingKeys);
			for (unsigned int k = 0; k < ai_channel->mNumPositionKeys; k++) {
				aiVectorKey ai_posKey = ai_channel->mPositionKeys[k];
				channel.positionKeys[k].first = ai_posKey.mTime / ai_animation->mTicksPerSecond;
				channel.positionKeys[k].second = glm::vec3(ai_posKey.mValue.x, ai_posKey.mValue.y, ai_posKey.mValue.z);
			}
			for (unsigned int k = 0; k < ai_channel->mNumRotationKeys; k++) {
				aiQuatKey ai_rotKey = ai_channel->mRotationKeys[k];
				channel.rotationKeys[k].first = ai_rotKey.mTime / ai_animation->mTicksPerSecond;
				channel.rotationKeys[k].second = glm::quat(ai_rotKey.mValue.x, ai_rotKey.mValue.y, ai_rotKey.mValue.z, ai_rotKey.mValue.w);
			}
			for (unsigned int k = 0; k < ai_channel->mNumPositionKeys; k++) {
				aiVectorKey ai_scaleKey = ai_channel->mScalingKeys[k];
				channel.scaleKeys[k].first = ai_scaleKey.mTime / ai_animation->mTicksPerSecond;
				channel.scaleKeys[k].second = glm::vec3(ai_scaleKey.mValue.x, ai_scaleKey.mValue.y, ai_scaleKey.mValue.z);
			}
		}
	}

	return animationData;
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
			texture.loadFromFile(path);
			texture.type = (type == aiTextureType_DIFFUSE ? TextureType_diffuse : TextureType_specular);
			this->textureCache[path] = texture;
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
	return glm::quat(quat.x, quat.y, quat.z, quat.w);
}

Transform ModelLoader::aiToGame(aiMatrix4x4 transform)
{
	aiVector3D scaling, position;
	aiQuaternion rotation;
	transform.Decompose(scaling, rotation, position);
	return Transform(aiToGlm(position), aiToGlm(rotation), aiToGlm(scaling));
}