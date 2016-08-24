
#include "Label.h"
#include "Renderer/Texture.h"
#include "Renderer/RenderUtil.h"

#include <Windows.h>
#include <GL/glew.h>

#include <algorithm>

const unsigned Label::defaultSize = 64;

Label::Label(const std::shared_ptr<Font>& font)
	: Label(font, defaultSize)
{ }

Label::Label(const std::shared_ptr<Font>& font, unsigned maxSize)
	: font(font), maxSize(maxSize), nVertices(0), nIndices(0),
	text(""), alignment(Alignment_left)
{
	this->generateBuffers();
	material.setProperty("texture_diffuse", MaterialProperty(Texture(TextureType_diffuse, font->getTextureId())));
}

void Label::generateBuffers()
{
	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	glGenBuffers(1, &this->ebo);
	glCheckError();

	this->resizeBuffers();
}

void Label::resizeBuffers()
{
	glBindVertexArray(this->vao);
	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	// 4 verts per char, 4 floats per vert
	glBufferData(GL_ARRAY_BUFFER, this->maxSize * 4 * sizeof(GLfloat) * 4, NULL, GL_DYNAMIC_DRAW);
	glCheckError();

	std::vector<unsigned> indices(this->generateIndices());
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	glCheckError();

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glCheckError();

	glBindVertexArray(0);
	glCheckError();
}

void Label::setAlignment(const Alignment& alignment)
{
	if (this->alignment == alignment) {
		return;
	}

	this->alignment = alignment;
	generateTextMesh();
}

void Label::setText(const std::string& newText)
{
	if (this->text.compare(newText) == 0) {
		return;
	}

	this->text = newText;
	generateTextMesh();
}

void Label::generateTextMesh()
{
	if (text.size() == 0) {
		return;
	}

	float cursorPos = 0.0f;
	std::vector<glm::vec4> verts;
	for (unsigned i = 0; i < text.size(); i++) {
		char c = text[i];
		Character character = font->getCharacter(c);

		float xpos = (float)(cursorPos + character.bearing.x);
		float ypos = (float)(character.size.y - character.bearing.y);

		float w = (float)character.size.x;
		float h = (float)character.size.y;

		// Normalize texture coordinates
		float texLeft = character.origin.x / (float)font->getTextureSize().x;
		float texRight = (character.origin.x + character.size.x) / (float)font->getTextureSize().x;
		float texTop = character.origin.y / (float)font->getTextureSize().y;
		float texBot = (character.origin.y + character.size.y) / (float)font->getTextureSize().y;

		verts.push_back(glm::vec4(xpos,     ypos - h, texLeft, texTop));
		verts.push_back(glm::vec4(xpos + w, ypos - h, texRight, texTop));
		verts.push_back(glm::vec4(xpos,     ypos,     texLeft, texBot));
		verts.push_back(glm::vec4(xpos + w, ypos,     texRight, texBot));

		cursorPos += (character.advance >> 6);
	}

	glm::vec2 offset(0.0f);
	switch(alignment) {
	case Alignment_left:
		// nothing
		break;
	case Alignment_right:
		offset.x = -cursorPos;
		break;
	case Alignment_center:
		offset.x = -cursorPos/2.0f;
		break;
	}

	for (unsigned i = 0; i < verts.size(); i++) {
		verts[i].x += offset.x;
		verts[i].y += offset.y;
	}

	if (maxSize == 0) {
		maxSize = text.size();
		this->generateBuffers();
	} else if (maxSize < text.size()) {
		maxSize = text.size();
		this->resizeBuffers();
	}

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(GLfloat) * verts.size() * 4, verts.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glCheckError();

	this->nVertices = text.size() * 4;
	this->nIndices = text.size() * 6;
}

std::vector<unsigned> Label::generateIndices()
{
	std::vector<unsigned> indices;
	for (unsigned i = 0; i < this->maxSize; i++) {
		indices.push_back(i * 4 + 0);
		indices.push_back(i * 4 + 2);
		indices.push_back(i * 4 + 3);
		indices.push_back(i * 4 + 0);
		indices.push_back(i * 4 + 3);
		indices.push_back(i * 4 + 1);
	}
	return indices;
}

unsigned Label::getVao() const
{
	return this->vao;
}

unsigned Label::getIndexCount() const
{
	return this->nIndices;
}

const Material& Label::getMaterial() const
{
	return this->material;
}

glm::mat4 Label::getTransform() const
{
	return this->transform.matrix();
}
