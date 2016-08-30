#pragma once

#include "Renderable2d.h"
#include "HandlePool.h"
#include "Transform.h"

#include <glm/glm.hpp>
#include <memory>
#include <list>

/*! Represents a single UI element within the renderer. */
struct UIRendererEntity {
	/*! The thing to render. */
	std::shared_ptr<Renderable2d> renderable;

	/*! The shader which will be used to render the element. */
	unsigned shaderHandle;
};

/*! Renderer of UI elements. */
class UIRenderer
{
public:
	/*!
	 * \brief Default initializer.
	 */
	UIRenderer();

	/*!
	 * \brief Sets the projection matrix to use.
	 *	Usually an orthographic projection the size of the screen.
	 * \param projection The projection matrix.
	 */
	void setProjection(glm::mat4 projection);

	/*!
	 * \brief Creates a new UI element.
	 * \param renderable The new element to render.
	 * \param shader The shader to use when rendering the element.
	 * \return Handle which is used to refer to the element in subsequent calls.
	 */
	unsigned getEntityHandle(const std::shared_ptr<Renderable2d>& renderable, const Shader& shader);

	/*!
	 * \brief Draws all entities which have been passed to the renderer.
	 */
	void draw();
private:
	/*! Map of shader IDs to shaders. The key is the value's shaderID property. */
	std::unordered_map<unsigned, Shader> shaderMap;

	/*! Pool of elements which have been initialized. */
	HandlePool<UIRendererEntity> pool;

	/*! List of entities sorted by z position, used when drawing. */
	std::list<std::pair<uint32_t, UIRendererEntity>> sortedEntities;

	/*! Projection to use when drawing elements. */
	glm::mat4 projection;

	/*! Comparator used when sorting sortedEntities. */
	class UIRendererSortComparator
	{
	public:
		bool operator() (const std::pair<uint32_t, UIRendererEntity>& p1, const std::pair<uint32_t, UIRendererEntity>& p2);
	};

	/*! Instance of the comparator. */
	UIRendererSortComparator comparator;
};