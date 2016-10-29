
#include "Renderer/UI/UIRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Optional.h"

#include "Renderer/Shader.h"
#include "Renderer/Material.h"
#include "Renderer/RenderUtil.h"

/*! Represents a single UI element within the renderer. */
struct UIRenderer::Entity {
	/*! The thing to render. */
	std::shared_ptr<Renderable2d> renderable;

	/*! The shader which will be used to render the element. */
	ShaderImpl shader;
};

/*! Comparator used when sorting sortedEntities. */
class UIRendererSortComparator
{
public:
	bool operator() (const std::pair<HandlePool<UIRenderer::Entity>::WeakHandle, UIRenderer::Entity>& p1, const std::pair<HandlePool<UIRenderer::Entity>::WeakHandle, UIRenderer::Entity>& p2)
	{
		// 3,2 is the location of z position in the matrix
		return p1.second.renderable->getTransform()[3][2] < p2.second.renderable->getTransform()[3][2];
	}
};

struct UIRenderer::Impl
{
	/*! Pool of elements which have been initialized. */
	HandlePool<UIRenderer::Entity> pool;

	/*! List of entities sorted by z position, used when drawing. */
	std::list<std::pair<HandlePool<UIRenderer::Entity>::WeakHandle, UIRenderer::Entity>> sortedEntities;

	/*! Projection to use when drawing elements. */
	glm::mat4 projection;

	/*! Instance of the comparator. */
	UIRendererSortComparator comparator;
};

UIRenderer::UIRenderer()
	: impl(new Impl())
{ }

UIRenderer::~UIRenderer()
{ }

void UIRenderer::setProjection(glm::mat4 projection)
{
	this->impl->projection = projection;
}

UIRenderer::UIElementHandle UIRenderer::getEntityHandle(const std::shared_ptr<Renderable2d>& renderable, const Shader& shader)
{
	Entity entity;
	entity.renderable = renderable;
	entity.shader = *shader.impl;

	UIElementHandle handle = impl->pool.getNewHandle(entity);
	HandlePool<UIRenderer::Entity>::WeakHandle weakHandle(handle);
	impl->sortedEntities.push_back(std::make_pair(weakHandle, entity));

	return handle;
}

void UIRenderer::draw()
{
	// We can't sort only when an element is added - elements' transforms might change
	// without us knowing. That being said, this is going to be slow for large amounts
	// of UI elements. Right now we don't have that many...
	impl->sortedEntities.sort(impl->comparator);

	glDisable(GL_DEPTH_TEST);

	auto iter = impl->sortedEntities.begin();
	while (iter != impl->sortedEntities.end())
	{
		if (iter->first.expired()) {
			iter = impl->sortedEntities.erase(iter);
			continue;
		}

		Entity& entity = iter->second;
		assert(entity.renderable != NULL);

		if (!entity.renderable->getIsVisible()) {
			++iter;
			continue;
		}

		const Renderable2d& renderable = *entity.renderable;
		const ShaderImpl& shader = entity.shader;
		const Material& material = renderable.getMaterial();

		shader.use();
		shader.setModelMatrix(renderable.getTransform());
		shader.setProjectionMatrix(impl->projection);
		shader.setViewMatrix(glm::mat4());
		
		material.apply(shader);

		glBindVertexArray(renderable.getVao());
		glDrawElements(glDrawTypeFromMaterial(material), renderable.getIndexCount(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glCheckError();

		++iter;
	}

	glEnable(GL_DEPTH_TEST);
}
