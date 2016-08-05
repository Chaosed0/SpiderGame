#pragma once

#include "Font.h"

#include <memory>

class Label
{
public:
	Label(const std::shared_ptr<Font>& font);
	Label(const std::shared_ptr<Font>& font, unsigned maxSize);

	void setText(const std::string& newText);

	unsigned getVao();
	unsigned getIndexCount();
private:
	std::shared_ptr<Font> font;
	unsigned vao, vbo, ebo;
	unsigned nVertices, nIndices;
	unsigned maxSize;

	std::vector<unsigned> generateIndices();
	void generateBuffers();
};