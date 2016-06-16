#pragma once

#include <Windows.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#include <glm/glm.hpp>

#include <unordered_map>
#include <map>

#include "Model.h"
#include "Material.h"
#include "Camera.h"
#include "RenderUtil.h"

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

/*! A renderable, stored internally in the renderer.
	Renderables can be equated to entities. They reference a model and a shader, but have
	their own transforms. */
struct Renderable
{
	Renderable(const Shader& shader, const Model& model, Transform transform)
		: shader(shader), model(model), transform(transform) { }

	const Shader& shader;
	const Model& model;
	Transform transform;

	/*! Current animation playing. */
	std::string animName;
	
	/*! Current time within the animation. */
	float time;
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
	unsigned int maxPointLights();

	/*!
	 * \brief Gets a handle to a renderable object.
	 */
	unsigned int getHandle(const Model& model, const Shader& shader);

	/*!
	 * \brief Updates the transform of a renderable object.
	 */
	void updateTransform(unsigned int handle, const Transform& transform);

	/*!
	 * \brief Updates the animation of a renderable object.
	 *		The animation will loop.
	 */
	void setAnimation(unsigned int handle, const std::string& animation);

	/*!
	 * \brief Draws all renderable objects requested using getHandle.
	 */
	void draw();

	/*!
	 * \brief Steps the animations of all renderable objects by the given amount.
	 * \param dt Step, in seconds.
	 */
	void update(float dt);
private:
	/*! Reference to the current camera. */
	Camera* camera;

	/*! The global directional light. */
	DirLight dirLight;

	/*! Point lights. */
	std::vector<PointLight> pointLights;

	/*! Map of shader IDs to Shaders. */
	std::map<unsigned int, Shader> shaderMap;

	/*! Map of model IDs to Models. */
	std::unordered_map<unsigned int, Model> modelMap;
	
	/*! Map of renderable handles to Renderables. */
	std::map<unsigned int, Renderable> renderableMap;

	/*! The callback to call when an OpenGL debug message is emitted. */
	DebugLogCallback debugLogCallback;

	/*! ID to assign to the next renderable requested through getHandle(). */
	unsigned int nextId;
};