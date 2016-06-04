
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
		std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		std::vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> ModelLoader::loadMaterialTextures(aiMaterial* mat, aiTextureType type)
{
	std::vector<Texture> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		std::string path(str.C_Str());

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