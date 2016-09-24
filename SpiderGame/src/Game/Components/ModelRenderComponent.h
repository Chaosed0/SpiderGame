#pragma once

#include "Framework/Component.h"
#include "Renderer/Renderer.h"

struct ModelRenderComponent : public Component
{
	ModelRenderComponent() : rendererHandle(nullptr) { }
	Renderer::RenderableHandle rendererHandle;
};