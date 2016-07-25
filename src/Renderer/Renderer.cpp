
#include "Renderer.h"
#include "RenderUtil.h"

#include <algorithm>
#include <sstream>
#include <ctime>

static const unsigned int maxPointLights = 4;
static const unsigned int maxBones = 100;

Renderer::Renderer()
	: pointLights(maxPointLights), camera(nullptr),
	nextRenderableHandle(0), nextModelHandle(0)
{ }

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

unsigned Renderer::getModelHandle(const Model& model)
{
	unsigned handle = this->nextModelHandle;
	modelMap.emplace(handle, model);
	this->nextModelHandle++;
	return handle;
}

unsigned Renderer::getRenderableHandle(unsigned modelHandle, const Shader& shader)
{
	auto shaderIter = shaderMap.find(shader.getID());
	if (shaderIter == shaderMap.end()) {
		auto iterPair = shaderMap.emplace(std::make_pair(shader.getID(), shader));
		shaderIter = iterPair.first;
	}

	auto modelIter = modelMap.find(modelHandle);
	if (modelIter == modelMap.end()) {
		return 0;
	}
	bool animatable = (modelIter->second.animationData.animations.size() > 0);

	unsigned int handle = this->nextRenderableHandle;
	// Index modelMap to initialize this so we don't depend on the passed reference
	renderableMap.emplace(std::make_pair(handle, Renderable(shader, modelHandle, animatable)));
	this->nextRenderableHandle++;
	return handle;
}

void Renderer::freeRenderableHandle(unsigned renderableHandle)
{
	renderableMap.erase(renderableHandle);
}

void Renderer::setRenderableTransform(unsigned handle, const Transform& transform)
{
	auto iter = renderableMap.find(handle);
	if (iter != renderableMap.end()) {
		iter->second.transform = transform;
	}
}

void Renderer::setRenderableAnimation(unsigned handle, const std::string& animName, bool loop)
{
	auto iter = renderableMap.find(handle);
	if (iter == renderableMap.end()) {
		return;
	}

	iter->second.animName = animName;
	iter->second.time = 0.0f;
	iter->second.loopAnimation = loop;
}

void Renderer::setRenderableAnimationTime(unsigned handle, float time)
{
	auto iter = renderableMap.find(handle);
	if (iter == renderableMap.end()) {
		return;
	}

	iter->second.time = time;
}

void Renderer::update(float dt)
{
	for (auto iter = renderableMap.begin(); iter != renderableMap.end(); iter++) {
		Renderable& renderable = iter->second;
		std::string animName = iter->second.animName;

		if (animName.size() == 0) {
			// Not being animated
			continue;
		}
		renderable.time += dt;

		Model& model = modelMap[iter->second.modelHandle];
		auto& animationMap = model.animationData.animations;
		auto animIter = animationMap.find(animName);

		if (animIter == animationMap.end()) {
			continue;
		}
		Animation& animation = animIter->second;

		float duration = animation.endTime - animation.startTime;
		if (renderable.time > duration) {
			// Loop or clamp
			if (renderable.loopAnimation) {
				renderable.time -= duration;
			} else {
				renderable.time = duration;
			}
		}
	}
}

void Renderer::draw()
{
	for (auto iter = shaderMap.begin(); iter != shaderMap.end(); iter++) {
		const ShaderCache& shaderCache = iter->second;

		shaderCache.shader.use();
		shaderCache.shader.setProjectionMatrix(&this->camera->getProjectionMatrix()[0][0]);
		shaderCache.shader.setViewMatrix(&this->camera->getViewMatrix()[0][0]);

		for (unsigned int i = 0; i < pointLights.size(); i++) {
			PointLight light = pointLights[i];
			glUniform1f(shaderCache.pointLights[i].constant, light.constant);
			glUniform1f(shaderCache.pointLights[i].linear, light.linear);
			glUniform1f(shaderCache.pointLights[i].quadratic, light.quadratic);
			glUniform3f(shaderCache.pointLights[i].ambient, light.ambient.x, light.ambient.y, light.ambient.z);
			glUniform3f(shaderCache.pointLights[i].diffuse, light.diffuse.x, light.diffuse.y, light.diffuse.z);
			glUniform3f(shaderCache.pointLights[i].specular, light.specular.x, light.specular.y, light.specular.z);
			glUniform3f(shaderCache.pointLights[i].position, light.position.x, light.position.y, light.position.z);
			glCheckError();
		}
		
		glUniform3f(shaderCache.dirLight.direction, dirLight.direction.x, dirLight.direction.y, dirLight.direction.z);
		glUniform3f(shaderCache.dirLight.ambient, dirLight.ambient.x, dirLight.ambient.y, dirLight.ambient.z);
		glUniform3f(shaderCache.dirLight.diffuse, dirLight.diffuse.x, dirLight.diffuse.y, dirLight.diffuse.z);
		glUniform3f(shaderCache.dirLight.specular, dirLight.specular.x, dirLight.specular.y, dirLight.specular.z);
		glCheckError();
	}

	// Render each renderable we have loaded through getHandle
	for (auto iter = renderableMap.begin(); iter != renderableMap.end(); iter++) {
		Renderable& renderable = iter->second;
		Model& model = modelMap[renderable.modelHandle];
		ShaderCache& shaderCache = renderable.shaderCache;
		Transform transform = renderable.transform;

		shaderCache.shader.use();
		shaderCache.shader.setModelMatrix(&transform.matrix()[0][0]);

		if (renderable.animatable) {
			std::vector<glm::mat4> nodeTransforms = model.getNodeTransforms(renderable.animName, renderable.time, renderable.context);

			for (unsigned i = 0; i < model.meshes.size(); i++) {
				Mesh& mesh = model.meshes[i];
				std::vector<glm::mat4> boneTransforms = mesh.getBoneTransforms(nodeTransforms);
				for (unsigned int j = 0; j < boneTransforms.size(); j++) {
					glUniformMatrix4fv(shaderCache.bones[j], 1, GL_FALSE, &boneTransforms[j][0][0]);
				}
			}
		}

		for (unsigned i = 0; i < model.meshes.size(); i++) {
			const Mesh& mesh = model.meshes[i];
			mesh.material.apply(shaderCache.shader);
			
			glBindVertexArray(mesh.VAO);
			glDrawElements(mesh.material.drawType, mesh.nIndices, GL_UNSIGNED_INT, 0);
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

unsigned int Renderer::getMaxPointLights()
{
	return pointLights.size();
}

ShaderCache::ShaderCache(const Shader& shader)
	: shader(shader),
	pointLights(maxPointLights),
	bones(maxBones)
{
	for (unsigned int i = 0; i < pointLights.size(); i++) {
		PointLightCache& light = this->pointLights[i];
		std::stringstream sstream;
		sstream << "pointLight[" << i << "]";
		light.constant = shader.getUniformLocation((sstream.str() + ".constant").c_str());
		light.linear = shader.getUniformLocation((sstream.str() + ".linear").c_str());
		light.quadratic = shader.getUniformLocation((sstream.str() + ".quadratic"));
		light.ambient = shader.getUniformLocation((sstream.str() + ".ambient").c_str());
		light.diffuse = shader.getUniformLocation((sstream.str() + ".diffuse").c_str());
		light.specular = shader.getUniformLocation((sstream.str() + ".specular").c_str());
		light.position = shader.getUniformLocation((sstream.str() + ".position").c_str());
		glCheckError();
	}
	
	this->dirLight.direction = shader.getUniformLocation("dirLight.direction");
	this->dirLight.ambient = shader.getUniformLocation("dirLight.ambient");
	this->dirLight.diffuse = shader.getUniformLocation("dirLight.diffuse");
	this->dirLight.specular = shader.getUniformLocation("dirLight.specular");

	for (unsigned int i = 0; i < maxBones; i++) {
		std::stringstream sstream;
		sstream << "bones[" << i << "]";
		this->bones[i] = shader.getUniformLocation(sstream.str());
	}
}