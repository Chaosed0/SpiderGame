#pragma once

#include "Renderer/Material.h"
#include "Transform.h"
#include <glm/glm.hpp>

struct Renderable2d
{
	virtual unsigned getVao() const = 0;
	virtual unsigned getIndexCount() const = 0;
	virtual Transform getTransform() const = 0;
	virtual const Material& getMaterial() const = 0;
};