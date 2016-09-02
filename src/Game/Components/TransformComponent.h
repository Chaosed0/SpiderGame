#pragma once

#include "Framework/Component.h"
#include "Transform.h"

struct TransformComponent : public Component
{
	TransformComponent(const Transform& transform) : transform(new Transform(transform)) { }
	TransformComponent() : transform(new Transform()) { }
	std::shared_ptr<Transform> transform;
};