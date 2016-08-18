
#include "Image.h"
#include "Renderer/RenderUtil.h"

#include <Windows.h>
#include <GL/glew.h>

#include <algorithm>

Image::Image(const Texture& texture, const glm::vec2& size)
{
	initialize(texture, size);
}

void Image::initialize(const Texture& texture, const glm::vec2& size)
{
	material.setProperty("texture_diffuse", texture);

	std::vector<glm::vec4> verts;
	verts.push_back(glm::vec4(0.0f, 0.0f, 0.0f, 0.0f));
	verts.push_back(glm::vec4(size.x, 0.0f, 1.0f, 0.0f));
	verts.push_back(glm::vec4(0.0f, size.y, 0.0f, 1.0f));
	verts.push_back(glm::vec4(size.x, size.y, 1.0f, 1.0f));

	std::vector<unsigned> indices;
	indices.push_back(0);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(0);
	indices.push_back(3);
	indices.push_back(1);

	glGenVertexArrays(1, &this->vao);
	glGenBuffers(1, &this->vbo);
	glGenBuffers(1, &this->ebo);
	glCheckError();

	glBindVertexArray(this->vao);

	glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat) * 4, verts.data(), GL_STATIC_DRAW);
	glCheckError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	glCheckError();

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glCheckError();

	glBindVertexArray(0);
	glCheckError();
}

unsigned Image::getVao() const
{
	return this->vao;
}

unsigned Image::getIndexCount() const
{
	return 6;
}

const Material& Image::getMaterial() const
{
	return this->material;
}

glm::mat4 Image::getTransform() const
{
	return this->transform.matrix();
}
