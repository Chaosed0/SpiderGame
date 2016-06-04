#pragma once

#include "Framework/Component.h"
#include "Renderer/Model.h"
#include "Renderer/Shader.h"

struct ModelRenderComponent : public Component
{
	unsigned int rendererHandle;
};