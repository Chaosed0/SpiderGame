#pragma once

#include <string>
#include <memory>

class ShaderImpl;

struct Shader
{
	Shader();
	std::unique_ptr<ShaderImpl> impl;
};

class ShaderLoader
{
public:
	Shader compileAndLink(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
private:
};