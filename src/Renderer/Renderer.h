#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <glm/glm.hpp>

#include <unordered_map>
#include <map>
#include <fstream>

#include "Model.h"
#include "Material.h"
#include "Camera.h"
#include "RenderUtil.h"
#include "Shader.h"
#include "HandlePool.h"

/*! A point light in space. */
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

/*! A global directional light. */
struct DirLight
{
    glm::vec3 direction;
  
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};  

/*! Shader cache. Stores a shader along with its uniform locations. */
struct ShaderCache
{
	ShaderCache(const Shader& shader);

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

	Shader shader;
	DirLightCache dirLight;
	std::vector<PointLightCache> pointLights;
	std::vector<GLuint> bones;
};

/*! Different render spaces. */
enum RenderSpace
{
	/*! Render in worldspace. */
	RenderSpace_World,

	/*! Render in UI space. */
	RenderSpace_UI
};

/*! A renderable, stored internally in the renderer.
	Renderables can be equated to entities. They reference a model and a shader, but have
	their own transforms. */
struct RendererEntity
{
	RendererEntity(const ShaderCache& shaderCache, uint32_t modelHandle, bool animatable)
		: shaderCache(shaderCache), modelHandle(modelHandle), animatable(animatable), space(RenderSpace_World) { }

	uint32_t modelHandle;
	ShaderCache shaderCache;
	Transform transform;
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

class Renderer
{
public:
	Renderer();

	/*!
	 * \brief Initializes the renderer. Must be called after GL initialization, and before draw() is called.
	 */
	bool initialize();

	/*!
	 * \brief Sets a callback to call in the event of an opengl debug message.
	 */
	void setDebugLogCallback(const DebugLogCallback& callback);

	/*!
	 * \brief Sets the current camera which this renderer will use.
	 */
	void setCamera(Camera* camera);

	/*!
	 * \brief Sets the directional light. Currently, only one is supported.
	 */
	void setDirLight(DirLight dirLight);

	/*!
	 * \brief Sets a point light.
	 * \param index Must be less than maxPointLights().
	 */
	void setPointLight(unsigned int index, PointLight pointLight);

	/*!
	 * \ brief Gets the maximum point lights allowed by the renderer.
	 */
	unsigned getMaxPointLights();

	/*!
	 * \brief Gets a handle to a model object, which can be passed to the getRenderableHandle method.
	 * The model is copied and stored internally, so you can release the reference to it.
	 */
	uint32_t getModelHandle(const Model& model);

	/*!
	 * \brief Gets a handle to a renderable object.
	 */
	uint32_t getRenderableHandle(uint32_t modelHandle, const Shader& shader);

	/*!
	 * \brief Frees a renderable handle. The renderable will stop rendering and
	 *		the handle will become unusable.
	 */
	void freeRenderableHandle(uint32_t renderableHandle);

	/*!
	 * \brief Updates the transform of a renderable object.
	 */
	void setRenderableTransform(uint32_t handle, const Transform& transform);

	/*!
	 * \brief Updates the animation of a renderable object.
	 *		The animation will loop.
	 */
	void setRenderableAnimation(uint32_t handle, const std::string& animation, bool loop = true);

	/*! 
	 * \brief Sets the time at which to start the currently playing
	 *		animation for the given object.
	 */
	void setRenderableAnimationTime(uint32_t handle, float time);

	/*! 
	 * \brief Sets the space in which the renderable will be rendered.
	 */
	void setRenderableRenderSpace(uint32_t handle, RenderSpace space);

	/*!
	 * \brief Draws all renderable objects that have been requested using getHandle()
	 *		and that haven't been freed yet.
	 */
	void draw();

	/*!
	 * \brief Steps the animations of all renderable objects by the given amount.
	 * \param dt Step, in seconds.
	 */
	void update(float dt);
private:
	/*!
	 * \brief Draws all renderable objects in a certain render space.
	 */
	void drawInternal(RenderSpace space);

	/*! Reference to the current camera. */
	Camera* camera;

	/*! The global directional light. */
	DirLight dirLight;

	/*! Point lights. */
	std::vector<PointLight> pointLights;

	/*! Map of shader ids to shaders. */
	std::unordered_map<unsigned, ShaderCache> shaderMap;
	
	/*! Pool for model objects registered with this renderer. */
	HandlePool<Model> modelPool;
	
	/*! Pool for renderables registered with this renderer. */
	HandlePool<RendererEntity> entityPool;

	/*! The callback to call when an OpenGL debug message is emitted. */
	DebugLogCallback debugLogCallback;

	/*! ID to assign to the next model requested through getModelHandle(). */
	unsigned nextModelHandle;

	/*! Model space transformation for UI space drawing. */
	glm::mat4 uiModelTransform;
};