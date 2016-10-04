#pragma once

#include "Framework/Component.h"
#include "Framework/DefaultComponentConstructor.h"
#include "Transform.h"

struct TransformComponent : public Component
{
	TransformComponent(const Transform& transform) : data(new Transform(transform)) { }
	TransformComponent() : data(new Transform()) { }
	std::shared_ptr<Transform> data;
};

class TransformConstructor : public ComponentConstructor {
public:
	TransformConstructor() { }
	TransformConstructor(const Transform& transform) : transform(transform) { }

	virtual ComponentConstructorInfo construct() const
	{
		TransformComponent* component = new TransformComponent(transform);
		return ComponentConstructorInfo(component, typeid(TransformComponent).hash_code());
	}
private:
	Transform transform;
};