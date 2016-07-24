#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include <glm/glm.hpp>

#include "Texture.h"
#include "Shader.h"

enum MaterialPropertyType
{
	MaterialPropertyType_vec3 = 0,
	MaterialPropertyType_vec4,
	MaterialPropertyType_texture,
	MaterialPropertyType_float,
	MaterialPropertyType_invalid
};

union MaterialPropertyValue
{
	MaterialPropertyValue() { memset(this, 0, sizeof(MaterialPropertyValue)); }
	MaterialPropertyValue(glm::vec3 vec3) : vec3(vec3) { }
	MaterialPropertyValue(glm::vec4 vec4) : vec4(vec4) { }
	MaterialPropertyValue(float flt) : flt(flt) { }
	MaterialPropertyValue(Texture texture) : texture(texture) { }

	glm::vec3 vec3;
	glm::vec4 vec4;
	float flt;
	Texture texture;
};

struct MaterialProperty
{
	MaterialProperty() : type(MaterialPropertyType_invalid) { }
	MaterialProperty(glm::vec3 vec3) : type(MaterialPropertyType_vec3), value(vec3) { }
	MaterialProperty(glm::vec4 vec4) : type(MaterialPropertyType_vec4), value(vec4) { }
	MaterialProperty(float flt) : type(MaterialPropertyType_float), value(flt) { }
	MaterialProperty(Texture texture) : type(MaterialPropertyType_texture), value(texture) { }

	MaterialPropertyType type;
	MaterialPropertyValue value;
	// By convention, the max of this field is 56 bytes
	uint64_t propertyId;
};

class Material
{
public:
	Material();
	MaterialProperty getProperty(const std::string& key);
	void setProperty(const std::string& key, MaterialProperty property);
	void apply(const Shader& shader) const;

	unsigned drawOrder;
	unsigned drawType;
private:
	std::map<std::string, MaterialProperty> properties;

	static uint64_t nextId;
	static std::unordered_map<uint64_t, GLuint> shaderUniformCache;
};