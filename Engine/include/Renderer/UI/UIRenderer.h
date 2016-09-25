#pragma once

#include "Renderable2d.h"
#include "HandlePool.h"

#include <glm/glm.hpp>
#include <memory>
#include <list>

/*! Represents a single UI element within the renderer. */
struct UIRendererEntity {
	/*! The thing to render. */
	std::shared_ptr<Renderable2d> renderable;

	/*! The shader which will be used to render the element. */
	uint64_t shaderHandle;
};

/*! Renderer of UI elements. */
class UIRenderer
{
public:
	typedef HandlePool<UIRendererEntity>::Handle UIElementHandle;

	/*!
	 * \brief Default initializer.
	 */
	UIRenderer();
	~UIRenderer();

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
	 UIElementHandle getEntityHandle(const std::shared_ptr<Renderable2d>& renderable, const Shader& shader);

	/*!
	 * \brief Draws all entities which have been passed to the renderer.
	 */
	void draw();
private:
	struct Impl;
	std::unique_ptr<Impl> impl;
};