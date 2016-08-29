
#include "UIRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Optional.h"

#include "Renderer/Shader.h"
#include "Renderer/Material.h"
#include "Renderer/RenderUtil.h"

bool UIRenderer::UIRendererSortComparator::operator() (const std::pair<uint32_t, UIRendererEntity>& p1, const std::pair<uint32_t, UIRendererEntity>& p2)
{
	return p1.second.renderable->getTransform().getPosition().z < p2.second.renderable->getTransform().getPosition().z;
}

UIRenderer::UIRenderer()
{ }

void UIRenderer::setProjection(glm::mat4 projection)
{
	this->projection = projection;
}

unsigned UIRenderer::getEntityHandle(const std::shared_ptr<Renderable2d>& renderable, const Shader& shader)
{
	auto iter = shaderMap.find(shader.getID());
	if (iter == shaderMap.end()) {
		auto iterPair = shaderMap.emplace(shader.getID(), shader);
		iter = iterPair.first;
	}

	UIRendererEntity entity;
	entity.renderable = renderable;
	entity.shaderHandle = shader.getID();

	uint32_t handle = pool.getNewHandle(entity);
	sortedEntities.push_back(std::make_pair(handle, entity));

	return handle;
}

void UIRenderer::draw()
{
	// We can't sort only when an element is added - elements' transforms might change
	// without us knowing. That being said, this is going to be slow for large amounts
	// of UI elements. Right now we don't have that many...
	sortedEntities.sort(comparator);

	for (auto iter = sortedEntities.begin(); iter != sortedEntities.end(); ++iter)
	{
		UIRendererEntity& entity = iter->second;
		assert(entity.renderable != NULL);

		const Renderable2d& renderable = *entity.renderable;
		const Shader& shader = shaderMap[entity.shaderHandle];
		const Material& material = renderable.getMaterial();

		shader.use();
		shader.setModelMatrix(&renderable.getTransform().matrix()[0][0]);
		shader.setProjectionMatrix(&projection[0][0]);
		shader.setViewMatrix(&glm::mat4()[0][0]);
		
		material.apply(shader);

		glBindVertexArray(renderable.getVao());
		glDrawElements(material.drawType, renderable.getIndexCount(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glCheckError();
	}
}
