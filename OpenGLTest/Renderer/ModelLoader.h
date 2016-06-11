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

	void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	AnimationData loadBoneData(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(const std::string& relDir, aiMaterial* mat, aiTextureType type);

	glm::vec3 aiToGlm(aiVector3D vec3);
	glm::quat aiToGlm(aiQuaternion quat);
	Transform aiToGame(aiMatrix4x4 transform);
};