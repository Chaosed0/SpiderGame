#pragma once

#include "Framework/Component.h"
#include "Renderer/Renderer.h"

struct ModelRenderComponent : public Component
{
	ModelRenderComponent() : rendererHandle(0), renderer(nullptr) { }
	~ModelRenderComponent()
	{
		if (renderer != nullptr) {
			renderer->freeRenderableHandle(rendererHandle);
		}
	}

	unsigned rendererHandle;
	Renderer* renderer;
};