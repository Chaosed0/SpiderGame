#pragma once

#include "Renderer/Material.h"
#include <glm/glm.hpp>

struct Renderable2d
{
	virtual unsigned getVao() const = 0;
	virtual unsigned getIndexCount() const = 0;
	virtual glm::mat4 getTransform() const = 0;
	virtual const Material& getMaterial() const = 0;
};