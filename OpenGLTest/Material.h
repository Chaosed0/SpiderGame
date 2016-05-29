#pragma once

#include <string>

#include <glm/glm.hpp>

#include "Texture.h"
#include "Shader.h"

enum MaterialPropertyType
{
	MaterialPropertyType_vec3 = 0,
	MaterialPropertyType_vec4,
	MaterialPropertyType_texture,
	MaterialPropertyType_float
};

union MaterialPropertyValue
{
	MaterialPropertyValue() { memset(this, 0, sizeof(MaterialPropertyValue)); }
	glm::vec3 vec3;
	glm::vec4 vec4;
	float flt;
	Texture texture;
};

struct MaterialProperty
{
	std::string key;
	MaterialPropertyType type;
	MaterialPropertyValue value;
};

class Material
{
public:
	Material();
	int drawOrder;
	void setProperty(MaterialProperty property);
	void apply(const Shader& shader) const;
private:
	std::vector<MaterialProperty> properties;
};