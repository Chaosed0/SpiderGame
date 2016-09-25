
#include "Shader.h"
#include "RenderUtil.h"

#include <fstream>
#include <vector>

ShaderImpl::ShaderImpl()
	: shaderID(0)
{ }

ShaderImpl::ShaderImpl(GLuint shaderID)
	: shaderID(shaderID)
{
	this->projectionUniform = this->getUniformLocation("projection");
	this->viewUniform = this->getUniformLocation("view");
	this->modelUniform = this->getUniformLocation("model");
}

void ShaderImpl::use() const
{
	glUseProgram(shaderID);
	glCheckError();
}

GLuint ShaderImpl::getID() const
{
	return shaderID;
}

GLint ShaderImpl::getUniformLocation(const std::string& uniformIdentifier) const
{
	return glGetUniformLocation(shaderID, uniformIdentifier.c_str());
}

void ShaderImpl::setViewMatrix(const glm::mat4& matrix) const
{
	glUniformMatrix4fv(viewUniform, 1, GL_FALSE, &matrix[0][0]);
	glCheckError();
}

void ShaderImpl::setProjectionMatrix(const glm::mat4& matrix) const
{
	glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, &matrix[0][0]);
	glCheckError();
}

void ShaderImpl::setModelMatrix(const glm::mat4& matrix) const
{
	glUniformMatrix4fv(modelUniform, 1, GL_FALSE, &matrix[0][0]);
	glCheckError();
}
