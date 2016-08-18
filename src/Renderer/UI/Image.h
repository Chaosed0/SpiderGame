#pragma once

#include "Font.h"
#include "Renderable2d.h"
#include "Transform.h"
#include "Renderer/Texture.h"

#include <memory>

class Image : public Renderable2d
{
public:
	Image(const Texture& texture, const glm::vec2& size);

	void initialize(const Texture& texture, const glm::vec2& size);

	virtual unsigned getVao() const;
	virtual unsigned getIndexCount() const;
	virtual const Material& getMaterial() const;
	virtual glm::mat4 getTransform() const;

	Material material;
	Transform transform;
private:
	Texture texture;
	unsigned vao, vbo, ebo;
};