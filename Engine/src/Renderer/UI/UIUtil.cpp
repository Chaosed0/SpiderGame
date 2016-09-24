
#include "Renderer/UI/UIUtil.h"
#include "Renderer/RenderUtil.h"

#include <vector>

#include <Windows.h>
#include <GL/glew.h>

UIUtilQuad UIUtil::generateQuad(const glm::vec2& size)
{
	UIUtilQuad quad;

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

	glGenVertexArrays(1, &quad.vao);
	glGenBuffers(1, &quad.vbo);
	glGenBuffers(1, &quad.ebo);
	glCheckError();

	glBindVertexArray(quad.vao);

	glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat) * 4, verts.data(), GL_STATIC_DRAW);
	glCheckError();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quad.ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
	glCheckError();

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
	glCheckError();

	glBindVertexArray(0);
	glCheckError();

	return quad;
}