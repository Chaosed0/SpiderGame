#pragma once

#include "Font.h"
#include "Renderable2d.h"
#include "Transform.h"

#include <memory>

class Label : public Renderable2d
{
public:
	Label(const std::shared_ptr<Font>& font);
	Label(const std::shared_ptr<Font>& font, unsigned maxSize);

	void setText(const std::string& newText);

	virtual unsigned getVao() const;
	virtual unsigned getIndexCount() const;
	virtual const Material& getMaterial() const;
	virtual glm::mat4 getTransform() const;

	Material material;
	Transform transform;
private:
	std::shared_ptr<Font> font;
	unsigned vao, vbo, ebo;
	unsigned nVertices, nIndices;
	unsigned maxSize;

	std::vector<unsigned> generateIndices();
	void generateBuffers();
	void resizeBuffers();

	const static unsigned defaultSize;
};