#pragma once

#include "Framework/Component.h"
#include "Transform.h"

struct TestMovementComponent : public Component
{
	TestMovementComponent() : initialized(false) {}
	bool initialized;
	Transform initialTransform;
};