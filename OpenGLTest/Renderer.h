#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <glm/glm.hpp>

#include <unordered_map>
#include <memory>
#include <map>

#include "Model.h"
#include "Material.h"
#include "Camera.h"

struct PointLight
{
    glm::vec3 position;

	float constant;
	float linear;
	float quadratic;
  
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct DirLight
{
    glm::vec3 direction;
  
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};  

struct Renderable
{
	Renderable(const Shader& shader, const Model& model, Transform transform)
		: shader(shader), model(model), transform(transform) { }
	const Shader& shader;
	const Model& model;
	Transform transform;
};

class Renderer
{
public:
	Renderer();

	void setCamera(std::shared_ptr<Camera> camera);

	void setDirLight(DirLight dirLight);
	void setPointLight(unsigned int index, PointLight pointLight);
	unsigned int maxPointLights();

	unsigned int getHandle(const Model& model, const Shader& shader);
	void updateTransform(unsigned int handle, const Transform& transform);
	void draw();
private:
	std::shared_ptr<Camera> camera;

	DirLight dirLight;
	std::vector<PointLight> pointLights;

	std::map<unsigned int, Shader> shaderMap;
	std::unordered_map<unsigned int, Model> modelMap;
	std::map<unsigned int, Renderable> renderableMap;

	unsigned int nextId;
};