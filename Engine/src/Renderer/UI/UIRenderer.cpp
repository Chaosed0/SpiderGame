
#include "Renderer/UI/UIRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Optional.h"

#include "Renderer/Shader.h"
#include "Renderer/Material.h"
#include "Renderer/RenderUtil.h"

/*! Comparator used when sorting sortedEntities. */
class UIRendererSortComparator
{
public:
	bool operator() (const std::pair<std::weak_ptr<uint32_t>, UIRendererEntity>& p1, const std::pair<std::weak_ptr<uint32_t>, UIRendererEntity>& p2)
	{
		// 2,3 is the location of z position in the matrix
		return p1.second.renderable->getTransform()[2][3] < p2.second.renderable->getTransform()[2][3];
	}
};

struct UIRenderer::Impl
{
	/*! Map of shader IDs to shaders. The key is the value's shaderID property. */
	std::unordered_map<uint64_t, ShaderImpl> shaderMap;

	/*! Pool of elements which have been initialized. */
	HandlePool<UIRendererEntity> pool;

	/*! List of entities sorted by z position, used when drawing. */
	std::list<std::pair<std::weak_ptr<uint32_t>, UIRendererEntity>> sortedEntities;

	/*! Projection to use when drawing elements. */
	glm::mat4 projection;

	/*! Instance of the comparator. */
	UIRendererSortComparator comparator;
};

UIRenderer::UIRenderer()
{ }

void UIRenderer::setProjection(glm::mat4 projection)
{
	this->impl->projection = projection;
}

UIRenderer::UIElementHandle UIRenderer::getEntityHandle(const std::shared_ptr<Renderable2d>& renderable, const Shader& shader)
{
	auto iter = impl->shaderMap.find(shader.impl->getID());
	if (iter == impl->shaderMap.end()) {
		auto iterPair = impl->shaderMap.emplace(shader.impl->getID(), *shader.impl);
		iter = iterPair.first;
	}

	UIRendererEntity entity;
	entity.renderable = renderable;
	entity.shaderHandle = shader.impl->getID();

	UIElementHandle handle = impl->pool.getNewHandle(entity);
	impl->sortedEntities.push_back(std::make_pair(handle, entity));

	return handle;
}

void UIRenderer::draw()
{
	// We can't sort only when an element is added - elements' transforms might change
	// without us knowing. That being said, this is going to be slow for large amounts
	// of UI elements. Right now we don't have that many...
	impl->sortedEntities.sort(impl->comparator);

	auto iter = impl->sortedEntities.begin();
	while (iter != impl->sortedEntities.end())
	{
		if (iter->first.expired()) {
			iter = impl->sortedEntities.erase(iter);
			continue;
		}

		UIRendererEntity& entity = iter->second;
		assert(entity.renderable != NULL);

		const Renderable2d& renderable = *entity.renderable;
		const ShaderImpl& shader = impl->shaderMap[entity.shaderHandle];
		const Material& material = renderable.getMaterial();

		shader.use();
		shader.setModelMatrix(renderable.getTransform());
		shader.setProjectionMatrix(impl->projection);
		shader.setViewMatrix(glm::mat4());
		
		material.apply(shader);

		glBindVertexArray(renderable.getVao());
		glDrawElements(material.drawType, renderable.getIndexCount(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glCheckError();

		++iter;
	}
}
