#pragma once

#include "Font.h"
#include "Renderable2d.h"
#include "Transform.h"
#include "Renderer/Texture.h"
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
	virtual Transform getTransform() const;

	Material material;
	Transform transform;
private:
	Texture texture;
	UIUtilQuad quad;
};