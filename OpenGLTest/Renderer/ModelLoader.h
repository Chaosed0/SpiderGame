#pragma once

#include <unordered_map>
#include <string>

#include <assimp/scene.h>

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

	std::unordered_map<std::string, Model> modelIdCache;
	std::unordered_map<std::string, Texture> textureCache;

	void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type);
};