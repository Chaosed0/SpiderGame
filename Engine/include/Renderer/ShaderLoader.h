#pragma once

#include <string>
#include <memory>

class ShaderImpl;

struct Shader
{
	Shader();
	Shader(const Shader& shader);
	~Shader();
	void operator=(const Shader& shader);
	bool isValid();
	std::unique_ptr<ShaderImpl> impl;
};

class ShaderLoader
{
public:
	Shader compileAndLink(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
private:
};