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

	/*!
	 * \brief Loads a model from the given path.
	 */
	Model loadModelFromPath(const std::string& path);

	/*!
	 * \brief Gets a model by an ID. The model must have been loaded by assignModelToId first.
	 */
	Model loadModelById(const std::string& id);

	/*!
	 * \brief Assigns an already-existing model to the id. Load the model later using loadModelById.
	 */
	void assignModelToId(const std::string& id, std::vector<Mesh> meshes);
private:
	/*! Next model ID to assign. */
	unsigned int nextId;

	/*! The relative directory to the model we are currently loading. */
	std::string curDir;

	/*! Cache mapping IDs to models. The IDs can also be paths. */
	std::unordered_map<std::string, Model> modelIdCache;

	/*! Cache mapping IDs to textures. The IDs are usually paths. */
	std::unordered_map<std::string, Texture> textureCache;

	/*!
	 * \brief Processes an assimp model, starting from its root node.
	 */
	Model processRootNode(aiNode* node, const aiScene* scene);

	/*!
	 * \brief Processes a specific mesh of a model.
	 * \param nodeIdMap A map of node names to internal node IDs. Used when the bones are being loaded from the mesh.
	 */
	Mesh processMesh(aiMesh* mesh, const aiScene* scene, std::unordered_map<std::string, unsigned int> nodeIdMap);

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