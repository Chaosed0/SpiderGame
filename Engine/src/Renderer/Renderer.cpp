
#include "Renderer/Renderer.h"
#include "Renderer/RenderUtil.h"
#include "Renderer/Shader.h"
#include "Renderer/Mesh.h"
#include "Renderer/MeshImpl.h"

#include "Optional.h"

#include <GL/glew.h>

#include <algorithm>
#include <sstream>
#include <ctime>

static const unsigned int maxPointLights = 64;
static const unsigned int maxBones = 100;

/*! Shader cache. Stores a shader along with its uniform locations. */
struct Renderer::ShaderCache
{
	ShaderCache(const ShaderImpl& shader);

	struct PointLightCache
	{
		GLuint position;

		GLuint constant;
		GLuint linear;
		GLuint quadratic;
	  
		GLuint ambient;
		GLuint diffuse;
		GLuint specular;
	};

	struct DirLightCache
	{
		GLuint direction;
		GLuint ambient;
		GLuint diffuse;
		GLuint specular;
	};  

	ShaderImpl shader;
	DirLightCache dirLight;
	std::vector<PointLightCache> pointLights;
	std::vector<GLuint> bones;
	GLuint pointLightCount;
};

/*! A renderable, stored internally in the renderer.
	Renderables can be equated to entities. They reference a model and a shader, but have
	their own transforms. */
struct Renderer::Entity
{
	Entity(const ShaderImpl& shader, HandlePool<Model>::Handle modelHandle, bool animatable)
		: shaderCache(shader), modelHandle(modelHandle), animatable(animatable), space(RenderSpace_World) { }

	HandlePool<Model>::Handle modelHandle;
	ShaderCache shaderCache;
	glm::mat4 transform;
	AnimationContext context;

	/*! Which space to render the renderable in. Defaults to RenderSpace_World. */
	RenderSpace space;

	/*! Whether or not this renderable can be animated. */
	bool animatable;

	/*! Current animation playing. */
	std::string animName;
	
	/*! Current time within the animation. */
	float time;

	/*! Whether or not to loop the current animation. */
	bool loopAnimation;
};

Renderer::Renderer()
	: pointLights(maxPointLights),
	camera(nullptr),
	pointLightCount(0)
{
	this->uiModelTransform = glm::mat4();
	// Flip the y axis so we can use normal modelspace but position in UI space
	this->uiModelTransform[1][1] = -1.0f;
}

Renderer::~Renderer()
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

Renderer::ModelHandle Renderer::getModelHandle(const Model& model)
{
	return modelPool.getNewHandle(model);
}

Renderer::RenderableHandle Renderer::getRenderableHandle(const ModelHandle& modelHandle, const Shader& shader)
{
	auto shaderIter = shaderMap.find(shader.impl->getID());
	if (shaderIter == shaderMap.end()) {
		auto iterPair = shaderMap.emplace(std::make_pair(shader.impl->getID(), ShaderCache(*shader.impl)));
		shaderIter = iterPair.first;
	}

	std::experimental::optional<std::reference_wrapper<Model>> modelOpt = modelPool.get(modelHandle);
	if (!modelOpt) {
		return entityPool.invalidHandle;
	}
	Model& model = *modelOpt;

	bool animatable = (model.animationData.animations.size() > 0);

	RenderableHandle handle = this->entityPool.getNewHandle(Entity(*shader.impl, modelHandle, animatable));
	return handle;
}

void Renderer::setRenderableTransform(const RenderableHandle& handle, const glm::mat4& transform)
{
	std::experimental::optional<std::reference_wrapper<Entity>> renderableOpt = entityPool.get(handle);
	if (renderableOpt) {
		Entity& renderable = *renderableOpt;
		renderable.transform = transform;
	}
}

void Renderer::setRenderableAnimation(const RenderableHandle& handle, const std::string& animName, bool loop)
{
	std::experimental::optional<std::reference_wrapper<Entity>> renderableOpt = entityPool.get(handle);
	if (!renderableOpt) {
		return;
	}

	Entity& renderable = *renderableOpt;
	renderable.animName = animName;
	renderable.time = 0.0f;
	renderable.loopAnimation = loop;
}

void Renderer::setRenderableAnimationTime(const RenderableHandle& handle, float time)
{
	std::experimental::optional<std::reference_wrapper<Entity>> renderableOpt = entityPool.get(handle);
	if (!renderableOpt) {
		return;
	}

	Entity& renderable = *renderableOpt;
	renderable.time = time;
}

void Renderer::setRenderableRenderSpace(const RenderableHandle& handle, RenderSpace space)
{
	std::experimental::optional<std::reference_wrapper<Entity>> renderableOpt = entityPool.get(handle);
	if (!renderableOpt) {
		return;
	}

	Entity& renderable = *renderableOpt;
	renderable.space = space;
}

void Renderer::update(float dt)
{
	for (auto iter = entityPool.begin(); iter != entityPool.end(); iter++) {
		Entity& renderable = iter->second;
		std::string animName = iter->second.animName;

		if (animName.size() == 0) {
			// Not being animated
			continue;
		}
		renderable.time += dt;

		std::experimental::optional<std::reference_wrapper<Model>> modelOpt = modelPool.get(iter->second.modelHandle);
		assert(modelOpt);

		Model& model = *modelOpt;
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
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_FRAMEBUFFER_SRGB);
	this->drawInternal(RenderSpace_World);

	glDisable(GL_DEPTH_TEST);
	this->drawInternal(RenderSpace_UI);
}

void Renderer::drawInternal(RenderSpace space)
{
	for (auto iter = shaderMap.begin(); iter != shaderMap.end(); iter++) {
		const ShaderCache& shaderCache = iter->second;

		shaderCache.shader.use();
		if (space == RenderSpace_World) {
			shaderCache.shader.setProjectionMatrix(this->camera->getProjectionMatrix());
			shaderCache.shader.setViewMatrix(this->camera->getViewMatrix());
		} else {
			shaderCache.shader.setProjectionMatrix(this->camera->getProjectionMatrixOrtho());
			shaderCache.shader.setViewMatrix(this->camera->getViewMatrixOrtho());
			continue;
		}
		glCheckError();

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
		glUniform1i(shaderCache.pointLightCount, pointLightCount);
		glCheckError();
	}

	// Render each renderable we have loaded through getHandle
	for (auto iter = entityPool.begin(); iter != entityPool.end(); iter++) {
		Entity& renderable = iter->second;
		if (renderable.space != space) {
			continue;
		}

		std::experimental::optional<std::reference_wrapper<Model>> modelOpt = modelPool.get(iter->second.modelHandle);
		assert(modelOpt);

		Model& model = *modelOpt;
		ShaderCache& shaderCache = renderable.shaderCache;
		glm::mat4 modelMatrix = renderable.transform;

		if (renderable.space == RenderSpace_UI) {
			modelMatrix = modelMatrix * this->uiModelTransform;
		}

		shaderCache.shader.use();

		if (renderable.animatable) {
			Mesh& mesh = model.mesh;
			std::vector<glm::mat4> nodeTransforms = model.getNodeTransforms(renderable.animName, renderable.time, renderable.context);
			if (renderable.animName.empty()) {
				// Just do bindpose
			} else if (mesh.impl->boneData.size() == 0) {
				// Not skinned animation
				// TODO: Actually find the node of the mesh
				modelMatrix *= nodeTransforms[1];
			} else {
				// Skinned animation
				std::vector<glm::mat4> boneTransforms = mesh.getBoneTransforms(nodeTransforms);
				for (unsigned int j = 0; j < boneTransforms.size(); j++) {
					glUniformMatrix4fv(shaderCache.bones[j], 1, GL_FALSE, &boneTransforms[j][0][0]);
				}
			}
		}

		shaderCache.shader.setModelMatrix(modelMatrix);
		model.material.apply(shaderCache.shader);

		const Mesh& mesh = model.mesh;
		glBindVertexArray(mesh.impl->VAO);
		glDrawElements(glDrawTypeFromMaterial(model.material), mesh.impl->nIndices, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glCheckError();
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

PointLight Renderer::getPointLight(unsigned int index)
{
	if (index < pointLights.size()) {
		return pointLights[index];
	}
	return PointLight();
}

void Renderer::setPointLight(unsigned int index, const PointLight& pointLight)
{
	if (index < pointLights.size()) {
		pointLights[index] = pointLight;
		pointLightCount = (std::max)(index + 1, pointLightCount);
	}
}

unsigned int Renderer::getMaxPointLights()
{
	return pointLights.size();
}

Renderer::ShaderCache::ShaderCache(const ShaderImpl& shader)
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

	this->pointLightCount = shader.getUniformLocation("pointLightCount");

	for (unsigned int i = 0; i < maxBones; i++) {
		std::stringstream sstream;
		sstream << "bones[" << i << "]";
		this->bones[i] = shader.getUniformLocation(sstream.str());
	}
}