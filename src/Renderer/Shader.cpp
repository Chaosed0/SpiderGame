
#include "Shader.h"
#include "RenderUtil.h"

#include <fstream>
#include <vector>

Shader::Shader()
	: shaderID(0)
{ }

Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	compileAndLink(vertexShaderPath, fragmentShaderPath);
}

void Shader::compileAndLink(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
{
	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertexShaderPath, std::ios::in);
	if (VertexShaderStream.is_open()) {
		std::string Line = "";
		while (getline(VertexShaderStream, Line)) {
			VertexShaderCode += "\n" + Line;
		}
		VertexShaderStream.close();
	} else {
		printf("Couldn't open shader file %s\n", vertexShaderPath.c_str());
		getchar();
		return;
	}

	// Read the Fragment Shader code from the file
	ready = true;
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragmentShaderPath, std::ios::in);
	if (FragmentShaderStream.is_open()) {
		std::string Line = "";
		while (getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertexShaderPath.c_str());
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		fprintf(stderr, "%s\n", &VertexShaderErrorMessage[0]);
		ready = false;
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragmentShaderPath.c_str());
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		fprintf(stderr, "%s\n", &FragmentShaderErrorMessage[0]);
		ready = false;
	}



	// Link the program
	printf("Linking program\n");
	this->shaderID = glCreateProgram();
	glAttachShader(shaderID, VertexShaderID);
	glAttachShader(shaderID, FragmentShaderID);
	glLinkProgram(shaderID);

	// Check the program
	glGetProgramiv(shaderID, GL_LINK_STATUS, &Result);
	glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(shaderID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		fprintf(stderr, "%s\n", &ProgramErrorMessage[0]);
		ready = false;
	}

	glDetachShader(shaderID, VertexShaderID);
	glDetachShader(shaderID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	this->projectionUniform = this->getUniformLocation("projection");
	this->viewUniform = this->getUniformLocation("view");
	this->modelUniform = this->getUniformLocation("model");
}

void Shader::use() const
{
	glUseProgram(shaderID);
	glCheckError();
}

bool Shader::isReady() const
{
	return ready;
}

GLuint Shader::getID() const
{
	return shaderID;
}

GLuint Shader::getUniformLocation(const std::string& uniformIdentifier) const
{
	return glGetUniformLocation(shaderID, uniformIdentifier.c_str());
}

void Shader::setViewMatrix(const GLfloat* matrix) const
{
	glUniformMatrix4fv(viewUniform, 1, GL_FALSE, matrix);
	glCheckError();
}

void Shader::setProjectionMatrix(const GLfloat* matrix) const
{
	glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, matrix);
	glCheckError();
}

void Shader::setModelMatrix(const GLfloat* matrix) const
{
	glUniformMatrix4fv(modelUniform, 1, GL_FALSE, matrix);
	glCheckError();
}
