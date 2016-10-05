#pragma once

#include "Transform.h"

struct PrefabConstructionInfo
{
	PrefabConstructionInfo() { }
	PrefabConstructionInfo(const Transform& transform) : initialTransform(transform) { }
	Transform initialTransform;
};