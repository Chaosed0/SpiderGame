#pragma once

#include "Font.h"
#include "Renderable2d.h"
#include "Renderer/TextureLoader.h"
#include "Renderer/UI/UIUtil.h"

#include <memory>

class UIQuad : public Renderable2d
{
public:
	UIQuad(const glm::vec2& size);
	UIQuad(const Texture& texture, const glm::vec2& size);
	UIQuad(const glm::vec4& color, const glm::vec2& size);

	virtual unsigned getVao() const;
	virtual unsigned getIndexCount() const;
	virtual const Material& getMaterial() const;
	virtual glm::mat4 getTransform() const;

	Material material;
	glm::mat4 transform;
private:
	Texture texture;
	UIUtilQuad quad;
};