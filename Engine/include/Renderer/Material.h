#pragma once

#include <string>
#include <map>
#include <unordered_map>

#include <glm/glm.hpp>

#include "TextureLoader.h"
#include "ShaderLoader.h"

enum MaterialDrawOrder
{
	MaterialDrawOrder_Less,
	MaterialDrawOrder_LEqual
};

enum MaterialDrawType
{
	MaterialDrawType_Triangles,
	MaterialDrawType_Lines
};

enum MaterialPropertyType
{
	MaterialPropertyType_vec3 = 0,
	MaterialPropertyType_vec4,
	MaterialPropertyType_texture,
	MaterialPropertyType_float,
	MaterialPropertyType_invalid
};

struct MaterialProperty
{
	MaterialProperty();
	~MaterialProperty();
	MaterialProperty(const MaterialProperty& property);
	MaterialProperty(glm::vec3 vec3);
	MaterialProperty(glm::vec4 vec4);
	MaterialProperty(float flt);
	MaterialProperty(const Texture& texture);

	union MaterialPropertyValue;
	MaterialPropertyType type;
	std::unique_ptr<MaterialPropertyValue> value;
	// By convention, the max of this field is 56 bytes
	uint64_t propertyId;
};

class Material
{
public:
	Material();
	void setProperty(const std::string& key, const MaterialProperty& property);
	void setTextures(const std::vector<Texture>& textures);
	void apply(const Shader& shader) const;
	void apply(const ShaderImpl& shader) const;

	MaterialDrawOrder drawOrder;
	MaterialDrawType drawType;
private:
	std::map<std::string, MaterialProperty> properties;
};