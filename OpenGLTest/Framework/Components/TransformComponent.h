#pragma once

#include "Framework/Component.h"
#include "Transform.h"

struct TransformComponent : public Component
{
	Transform transform;
};