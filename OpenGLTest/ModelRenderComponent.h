#pragma once

#include "Component.h"
#include "Model.h"
#include "Shader.h"

struct ModelRenderComponent : public Component
{
	unsigned int rendererHandle;
};