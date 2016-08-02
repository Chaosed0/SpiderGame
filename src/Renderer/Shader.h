#pragma once

#include <string>

#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

class Shader
{
public:
	Shader();
	Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

	void compileAndLink(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

	void use() const;
	bool isReady() const;
	GLuint getID() const;

	GLuint getUniformLocation(const std::string& uniformIdentifier) const;
	void setViewMatrix(const GLfloat* matrix) const;
	void setProjectionMatrix(const GLfloat* matrix) const;
	void setModelMatrix(const GLfloat* matrix) const;
private:
	bool ready;
	GLuint shaderID;

	GLuint projectionUniform, viewUniform, modelUniform;
};