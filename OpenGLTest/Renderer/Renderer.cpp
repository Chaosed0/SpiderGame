
#include "Renderer.h"
#include "RenderUtil.h"

#include <sstream>

static const unsigned int maxPointLights = 4;

Renderer::Renderer()
	: pointLights(4)
{
	nextId = 1;
}

void Renderer::setDebugLogCallback(const DebugLogCallback& callback)
{
	this->debugLogCallback = callback;
	GLint flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, (void*)&this->debugLogCallback);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
}

bool Renderer::initialize()
{
	glewExperimental = true;
	if (glewInit() != GLEW_OK)
	{
		fprintf(stderr, "Could not initialize GLEW\n");
		return false;
	}
	// Known bug in glew, clear the error flag
	glGetError();

	return true;
}

unsigned int Renderer::getHandle(const Model& model, const Shader& shader)
{
	auto modelIter = modelMap.find(model.id);
	auto shaderIter = shaderMap.find(shader.getID());

	if (modelIter == modelMap.end()) {
		modelMap[model.id] = model;
	}

	if (shaderIter == shaderMap.end()) {
		shaderMap[shader.getID()] = shader;
	}

	// Index modelMap to initialize this so we don't depend on the passed reference
	unsigned int id = nextId;
	renderableMap.emplace(std::make_pair(id, Renderable(shaderMap[shader.getID()], modelMap[model.id], Transform::identity)));
	nextId++;
	return id;
}

void Renderer::updateTransform(unsigned int handle, const Transform& transform)
{
	auto iter = renderableMap.find(handle);
	if (iter != renderableMap.end()) {
		iter->second.transform = transform;
	}
}

void Renderer::draw()
{
	for (auto iter = shaderMap.begin(); iter != shaderMap.end(); iter++) {
		const Shader& shader = iter->second;

		shader.use();
		shader.setProjectionMatrix(&this->camera->getProjectionMatrix()[0][0]);
		shader.setViewMatrix(&this->camera->getViewMatrix()[0][0]);

		for (unsigned int i = 0; i < pointLights.size(); i++) {
			PointLight light = pointLights[i];
			std::stringstream sstream;
			sstream << "pointLight[" << i << "]";
			glUniform1f(shader.getUniformLocation((sstream.str() + ".constant").c_str()), light.constant);
			glUniform1f(shader.getUniformLocation((sstream.str() + ".linear").c_str()), light.linear);
			glUniform1f(shader.getUniformLocation((sstream.str() + ".quadratic")), light.quadratic);
			glUniform3f(shader.getUniformLocation((sstream.str() + ".ambient").c_str()), light.ambient.x, light.ambient.y, light.ambient.z);
			glUniform3f(shader.getUniformLocation((sstream.str() + ".diffuse").c_str()), light.diffuse.x, light.diffuse.y, light.diffuse.z);
			glUniform3f(shader.getUniformLocation((sstream.str() + ".specular").c_str()), light.specular.x, light.specular.y, light.specular.z);
			glUniform3f(shader.getUniformLocation((sstream.str() + ".position").c_str()), light.position.x, light.position.y, light.position.z);
			glCheckError();
		}
		
		glUniform3f(shader.getUniformLocation("dirLight.direction"), dirLight.direction.x, dirLight.direction.y, dirLight.direction.z);
		glUniform3f(shader.getUniformLocation("dirLight.ambient"), dirLight.ambient.x, dirLight.ambient.y, dirLight.ambient.z);
		glUniform3f(shader.getUniformLocation("dirLight.diffuse"), dirLight.diffuse.x, dirLight.diffuse.y, dirLight.diffuse.z);
		glUniform3f(shader.getUniformLocation("dirLight.specular"), dirLight.specular.x, dirLight.specular.y, dirLight.specular.z);
		glCheckError();
	}

	for (auto iter = renderableMap.begin(); iter != renderableMap.end(); iter++) {
		const Model& model = iter->second.model;
		const Shader& shader = iter->second.shader;
		Transform transform = iter->second.transform;

		shader.use();
		shader.setModelMatrix(&transform.matrix()[0][0]);

		for (unsigned int i = 0; i < model.meshes.size(); i++) {
			Mesh mesh = model.meshes[i];
			mesh.material.apply(shader);
			glBindVertexArray(mesh.VAO);
			glDrawElements(GL_TRIANGLES, mesh.nIndices, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glCheckError();
		}
	}
}

void Renderer::setCamera(Camera* camera)
{
	this->camera = camera;
}

void Renderer::setDirLight(DirLight dirLight)
{
	this->dirLight = dirLight;
}

void Renderer::setPointLight(unsigned int index, PointLight pointLight)
{
	if (index < pointLights.size()) {
		pointLights[index] = pointLight;
	}
}

unsigned int Renderer::maxPointLights()
{
	return pointLights.size();
}