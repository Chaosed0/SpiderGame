#pragma once

#include <glm/glm.hpp>

#include <unordered_map>
#include <map>
#include <fstream>

#include "Model.h"
#include "Material.h"
#include "Camera.h"
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

/*! Different render spaces. */
enum RenderSpace
{
	/*! Render in worldspace. */
	RenderSpace_World,

	/*! Render in UI space. */
	RenderSpace_UI
};

class Renderer
{
public:
	struct Entity;
	typedef HandlePool<Model>::Handle ModelHandle;
	typedef HandlePool<Entity>::Handle RenderableHandle;
	typedef HandlePool<PointLight>::Handle PointLightHandle;
	typedef std::function<void(const std::string&)> DebugLogCallback;

	Renderer();
	~Renderer();

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
	 * \param handle Handle previously registered using getPointLightHandle.
	 * \param pointLight New light parameters to pass to the renderer.
	 */
	void setPointLight(const PointLightHandle& handle, const PointLight& pointLight);

	/*!
	 * \brief Gets a point light.
	 * \param light Initial point light parameters.
	 * \return New point light handle. If all handles go out of scope, the point light is destroyed.
	 */
	PointLightHandle getPointLightHandle(const PointLight& light);

	/*!
	 * \brief Gets the parameters of an existing point light.
	 */
	PointLight getPointLight(const PointLightHandle& handle);

	/*!
	 * \brief Gets a handle to a model object, which can be passed to the getRenderableHandle method.
	 * The model is copied and stored internally, so you can release the reference to it.
	 */
	ModelHandle getModelHandle(const Model& model);

	/*!
	 * \brief Gets a handle to a renderable object.
	 */
	RenderableHandle getRenderableHandle(const ModelHandle& modelHandle, const Shader& shader);

	/*!
	 * \brief Updates the transform of a renderable object.
	 */
	void setRenderableTransform(const RenderableHandle& handle, const glm::mat4& transform);

	/*!
	 * \brief Updates the animation of a renderable object.
	 *		The animation will loop.
	 */
	void setRenderableAnimation(const RenderableHandle& handle, const std::string& animation, bool loop = true);

	/*! 
	 * \brief Sets the time at which to start the currently playing
	 *		animation for the given object.
	 */
	void setRenderableAnimationTime(const RenderableHandle& handle, float time);

	/*! 
	 * \brief Sets the space in which the renderable will be rendered.
	 */
	void setRenderableRenderSpace(const RenderableHandle& handle, RenderSpace space);

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

	/*! Map of shader ids to shaders. */
	struct ShaderCache;
	std::unordered_map<uint64_t, ShaderCache> shaderMap;
	
	/*! Pool for model objects registered with this renderer. */
	HandlePool<Model> modelPool;
	
	/*! Pool for renderables registered with this renderer. */
	HandlePool<Entity> entityPool;
	
	/*! Pool for point lights registered with this renderer. */
	HandlePool<PointLight> pointLightPool;

	/*! The callback to call when an OpenGL debug message is emitted. */
	DebugLogCallback debugLogCallback;

	/*! ID to assign to the next model requested through getModelHandle(). */
	unsigned nextModelHandle;

	/*! Model space transformation for UI space drawing. */
	glm::mat4 uiModelTransform;
};