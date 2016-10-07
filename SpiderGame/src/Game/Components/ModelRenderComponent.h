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
	ModelRenderConstructor(Renderer& renderer, const Renderer::ModelHandle& modelHandle, const Shader& shader, const std::string& defaultAnimation = "")
		: renderer(renderer), modelHandle(modelHandle), shader(shader), defaultAnimation(defaultAnimation) { }

	virtual ComponentConstructorInfo construct(World& world, eid_t parent, void* userinfo) const
	{
		ModelRenderComponent* component = new ModelRenderComponent();
		component->rendererHandle = renderer.getRenderableHandle(modelHandle, shader);
		return ComponentConstructorInfo(component, typeid(ModelRenderComponent).hash_code());
	}

	virtual void finish(World& world, eid_t entity)
	{
		ModelRenderComponent* component = world.getComponent<ModelRenderComponent>(entity);
		renderer.setRenderableAnimation(component->rendererHandle, defaultAnimation, true);
	}
private:
	Renderer& renderer;
	Shader shader;
	Renderer::ModelHandle modelHandle;
	std::string defaultAnimation;
};