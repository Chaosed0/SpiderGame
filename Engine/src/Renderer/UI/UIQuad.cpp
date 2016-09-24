
#include "Renderer/UI/UIQuad.h"
#include "Renderer/RenderUtil.h"

#include <Windows.h>
#include <GL/glew.h>

#include <algorithm>

UIQuad::UIQuad(const glm::vec2& size)
{
	quad = UIUtil::generateQuad(size);
}

UIQuad::UIQuad(const Texture& texture, const glm::vec2& size)
	: UIQuad(size)
{
	material.setProperty("texture_diffuse", texture);
}

UIQuad::UIQuad(const glm::vec4& color, const glm::vec2& size)
	: UIQuad(size)
{
	material.setProperty("color", color);
}

unsigned UIQuad::getVao() const
{
	return this->quad.vao;
}

unsigned UIQuad::getIndexCount() const
{
	return 6;
}

const Material& UIQuad::getMaterial() const
{
	return this->material;
}

glm::mat4 UIQuad::getTransform() const
{
	return this->transform;
}
