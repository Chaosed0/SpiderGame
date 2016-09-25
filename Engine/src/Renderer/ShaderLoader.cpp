
#include "Renderer/ShaderLoader.h"
#include "Renderer/RenderUtil.h"
#include "Renderer/Shader.h"

#include <fstream>
#include <vector>

Shader::Shader() : impl(new ShaderImpl()) { }
Shader::Shader(const Shader& shader) : impl(new ShaderImpl(*shader.impl)) { }
Shader::~Shader() { }
void Shader::operator=(const Shader& shader)
{
	this->impl = std::make_unique<ShaderImpl>(*shader.impl);
}
bool Shader::isValid()
{
	return impl->getID() != 0;
}

Shader ShaderLoader::compileAndLink(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
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
		return Shader();
	}

	// Read the Fragment Shader code from the file
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
	}



	// Link the program
	printf("Linking program\n");
	GLuint shaderID = glCreateProgram();
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
	}

	glDetachShader(shaderID, VertexShaderID);
	glDetachShader(shaderID, FragmentShaderID);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	GLint uniformCount;
	glGetProgramiv(shaderID, GL_ACTIVE_UNIFORMS, &uniformCount);
	char buffer[128];
	for (GLint i = 0; i < uniformCount; i++) {
		GLsizei length;
		glGetActiveUniformName(shaderID, i, 128, &length, buffer);
		printf("%s\n", buffer);
	}

	std::unique_ptr<ShaderImpl> impl(new ShaderImpl(shaderID));
	Shader shader;
	shader.impl.swap(impl);

	return shader;
}
