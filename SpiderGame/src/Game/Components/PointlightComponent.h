#pragma once

#include "Framework/Component.h"
#include "Framework/ComponentConstructor.h"
#include "Renderer/Renderer.h"

struct PointLightComponent : public Component
{
	PointLightComponent() : PointLightComponent(nullptr) { }
	PointLightComponent(const Renderer::PointLightHandle& handle) : handle(handle) { }
	Renderer::PointLightHandle handle;
};

class PointLightConstructor : public ComponentConstructor
{
public:
	PointLightConstructor(Renderer& renderer, const PointLight& pointLight)
		: renderer(renderer), pointLightParams(pointLight) { }

	virtual ComponentConstructorInfo construct(World& world, eid_t parent, void* userinfo) const
	{
		PointLightComponent* component = new PointLightComponent();
		component->handle = renderer.getPointLightHandle(pointLightParams);
		return ComponentConstructorInfo(component, typeid(PointLightComponent).hash_code());
	}
private:
	Renderer& renderer;
	PointLight pointLightParams;
};
