#pragma once

#include <unordered_map>
#include <string>
#include <memory>

#include <glm/glm.hpp>

#include "Renderer/Model.h"

class ModelLoader
{
public:
	ModelLoader();
	~ModelLoader();

	/*!
	 * \brief Loads a model from the given path.
	 */
	Model loadModelFromPath(const std::string& path);

	/*!
	 * \brief If a material property is not found, then the loader will pull from this material.
	 */
	void setDefaultMaterialProperties(const Material& material);
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};