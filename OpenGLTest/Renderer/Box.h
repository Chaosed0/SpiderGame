#pragma once

#include "Model.h"
#include "Texture.h"

Mesh getBox(const std::vector<Texture>& textures);
Mesh getBox(const std::vector<Texture>& textures, glm::vec3 scale);
Mesh getSkybox(const std::vector<std::string>& skyboxTextures);