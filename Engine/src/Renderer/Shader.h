#pragma once

#include <string>
#include <glm/glm.hpp>

#include <GL/glew.h>

class ShaderImpl
{
public:
	ShaderImpl();
	ShaderImpl(GLuint shaderID);

	void use() const;
	GLuint getID() const;

	GLint getUniformLocation(const std::string& uniformIdentifier) const;
	void setViewMatrix(const glm::mat4& matrix) const;
	void setProjectionMatrix(const glm::mat4& matrix) const;
	void setModelMatrix(const glm::mat4& matrix) const;
private:
	GLuint shaderID;
	GLint projectionUniform, viewUniform, modelUniform;
};