#pragma once

#include "Renderer/Model.h"
#include "Renderer/TextureLoader.h"

#include <glm/gtc/quaternion.hpp>

Model getBox(const std::vector<Texture>& textures);
Model getBox(const std::vector<Texture>& textures, glm::vec3 scale);
Model getSkybox(const std::vector<std::string>& skyboxTextures);
Model getPlane(const std::vector<Texture>& textures, glm::vec3 ubasis, glm::vec3 vbasis, glm::vec2 dimensions, glm::vec2 textureOffset = glm::vec2(0.0f), glm::vec2 textureScale = glm::vec2(1.0f));
Model getDebugBoxMesh(const glm::vec3& halfExtents);