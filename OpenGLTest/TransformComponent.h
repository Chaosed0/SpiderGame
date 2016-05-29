#pragma once

#include "Component.h"
#include "Transform.h"

struct TransformComponent : public Component
{
	Transform transform;
};