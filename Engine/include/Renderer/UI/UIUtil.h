#pragma once

#include <glm/glm.hpp>

struct UIUtilQuad
{
	unsigned vao;
	unsigned vbo;
	unsigned ebo;
};

class UIUtil
{
public:
	static UIUtilQuad generateQuad(const glm::vec2& size);
};