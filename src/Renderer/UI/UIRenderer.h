#pragma once

#include "Renderer/HandlePool.h"
#include "Renderable2d.h"
#include "Transform.h"

#include <glm/glm.hpp>
#include <memory>
#include <list>

struct UIRendererEntity {
	std::shared_ptr<Renderable2d> renderable;
	unsigned shaderHandle;
};

class UIRenderer
{
public:
	UIRenderer();
	void setProjection(glm::mat4 projection);
	unsigned getEntityHandle(const std::shared_ptr<Renderable2d>& renderable, const Shader& shader);
	void draw();
private:
	std::unordered_map<unsigned, Shader> shaderMap;
	std::list<std::pair<uint32_t, UIRendererEntity>> sortedEntities;
	HandlePool<UIRendererEntity> pool;
	glm::mat4 projection;

	class UIRendererSortComparator
	{
	public:
		bool operator() (const std::pair<uint32_t, UIRendererEntity>& p1, const std::pair<uint32_t, UIRendererEntity>& p2);
	};
	UIRendererSortComparator comparator;
};