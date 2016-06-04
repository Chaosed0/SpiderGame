#pragma once

#include "Model.h"
#include "Texture.h"

Mesh getBox(const std::vector<Texture>& textures);
Mesh getSkybox(const std::vector<std::string>& skyboxTextures);