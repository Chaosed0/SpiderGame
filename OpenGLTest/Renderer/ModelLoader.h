#pragma once

#include <unordered_map>
#include <string>

#include <assimp/scene.h>

#include <glm/glm.hpp>

#include "Model.h"
#include "Texture.h"

class ModelLoader
{
public:
	ModelLoader();

	Model loadModelFromPath(const std::string& path);
	Model loadModelById(const std::string& id);
	void assignModelToId(const std::string& id, std::vector<Mesh> meshes);

private:
	unsigned int nextId;
	std::string curDir;

	std::unordered_map<std::string, Model> modelIdCache;
	std::unordered_map<std::string, Texture> textureCache;

	Model processRootNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::unordered_map<std::string, unsigned int> nodeIdMap);
	void ModelLoader::loadBoneData(aiMesh* mesh, const aiScene* scene, std::unordered_map<std::string, unsigned int> nodeIdMap, std::vector<VertexBoneData>& vertexBoneData, std::vector<BoneData>& boneData);
	std::vector<Texture> loadMaterialTextures(const std::string& relDir, aiMaterial* mat, aiTextureType type);

	glm::vec3 aiToGlm(aiVector3D vec3);
	glm::quat aiToGlm(aiQuaternion quat);
	glm::mat4 aiToGlm(aiMatrix4x4 mat4);
	glm::mat3 aiToGlm(aiMatrix3x3 mat3);
};