#pragma once

#include "Model.h"
#include "Texture.h"

#include <glm/gtc/quaternion.hpp>

Mesh getBox(const std::vector<Texture>& textures);
Mesh getBox(const std::vector<Texture>& textures, glm::vec3 scale);
Mesh getSkybox(const std::vector<std::string>& skyboxTextures);
Mesh getPlane(const std::vector<Texture>& textures, glm::quat basis, glm::vec2 dimensions, glm::vec2 textureOffset, glm::vec2 textureScale);
Mesh getDebugBoxMesh(const glm::vec3& halfExtents);