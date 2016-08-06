
#include "UIRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

#include "Optional.h"

#include "Renderer/Shader.h"
#include "Renderer/Material.h"
#include "Renderer/RenderUtil.h"

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

	return pool.getNewHandle(entity);
}

void UIRenderer::setTransform(uint32_t handle, const Transform& transform)
{
	std::experimental::optional<UIRendererEntity&> entity = this->pool.get(handle);
	if (!entity) {
		return;
	}

	entity->transform = transform;
}

void UIRenderer::draw()
{
	for (auto iter = pool.begin(); iter != pool.end(); ++iter)
	{
		UIRendererEntity& entity = iter->second;
		const Renderable2d& renderable = *entity.renderable;
		const Shader& shader = shaderMap[entity.shaderHandle];
		const Material& material = renderable.getMaterial();

		shader.use();
		material.apply(shader);
		shader.setModelMatrix(&entity.transform.matrix()[0][0]);
		shader.setProjectionMatrix(&projection[0][0]);
		shader.setViewMatrix(&glm::mat4()[0][0]);
		
		glBindVertexArray(renderable.getVao());
		glDrawElements(material.drawType, renderable.getIndexCount(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glCheckError();
	}
}
