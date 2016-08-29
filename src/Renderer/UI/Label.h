#pragma once

#include "Font.h"
#include "Renderable2d.h"
#include "Transform.h"

#include <memory>

class Label : public Renderable2d
{
public:
	enum Alignment
	{
		Alignment_left,
		Alignment_center,
		Alignment_right
	};

	Label();
	Label(const std::shared_ptr<Font>& font);
	Label(const std::shared_ptr<Font>& font, unsigned maxSize);

	void setAlignment(const Alignment& alignment);
	void setText(const std::string& newText);
	void setFont(const std::shared_ptr<Font>& font);

	virtual unsigned getVao() const;
	virtual unsigned getIndexCount() const;
	virtual const Material& getMaterial() const;
	virtual Transform getTransform() const;

	Material material;
	Transform transform;
private:
	std::shared_ptr<Font> font;
	unsigned vao, vbo, ebo;
	unsigned nVertices, nIndices;
	unsigned maxSize;

	std::string text;
	Alignment alignment;

	std::vector<unsigned> generateIndices();
	void generateBuffers();
	void resizeBuffers();
	void generateTextMesh();

	const static unsigned defaultSize;
};