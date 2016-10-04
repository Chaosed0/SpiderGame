#pragma once

#include "Framework/Component.h"
#include "Framework/ComponentConstructor.h"
#include "Renderer/Renderer.h"

struct ModelRenderComponent : public Component
{
	ModelRenderComponent() : rendererHandle(nullptr) { }
	Renderer::RenderableHandle rendererHandle;
};

class ModelRenderConstructor : public ComponentConstructor
{
public:
	ModelRenderConstructor(Renderer& renderer, const Renderer::ModelHandle& modelHandle, const Shader& shader)
		: renderer(renderer), modelHandle(modelHandle), shader(shader) { }

	virtual ComponentConstructorInfo construct() const
	{
		ModelRenderComponent* component = new ModelRenderComponent();
		component->rendererHandle = renderer.getRenderableHandle(modelHandle, shader);
		return ComponentConstructorInfo(component, typeid(ModelRenderComponent).hash_code());
	}

	Renderer::ModelHandle modelHandle;
private:
	Renderer& renderer;
	Shader shader;
};