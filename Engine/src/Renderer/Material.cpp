
#include "Renderer/Material.h"
#include "Renderer/RenderUtil.h"

#include <cassert>

#include "Renderer/Shader.h"
#include "Renderer/Texture.h"

static uint64_t nextId;
static std::unordered_map<uint64_t, GLint> shaderUniformCache;

union MaterialProperty::MaterialPropertyValue
{
	MaterialPropertyValue() { memset(this, 0, sizeof(MaterialPropertyValue)); }
	MaterialPropertyValue(glm::vec3 vec3) : vec3(vec3) { }
	MaterialPropertyValue(glm::vec4 vec4) : vec4(vec4) { }
	MaterialPropertyValue(float flt) : flt(flt) { }
	MaterialPropertyValue(const Texture& texture) : texture(*texture.impl) { }

	glm::vec3 vec3;
	glm::vec4 vec4;
	float flt;
	TextureImpl texture;
};

MaterialProperty::MaterialProperty() : type(MaterialPropertyType_invalid) { }
MaterialProperty::~MaterialProperty() { }
MaterialProperty::MaterialProperty(const MaterialProperty& property)
	: type(property.type),
	value(new MaterialPropertyValue(*property.value)),
	propertyId(property.propertyId)
{ }
MaterialProperty::MaterialProperty(glm::vec3 vec3) : type(MaterialPropertyType_vec3), value(new MaterialPropertyValue(vec3)) { }
MaterialProperty::MaterialProperty(glm::vec4 vec4) : type(MaterialPropertyType_vec4), value(new MaterialPropertyValue(vec4)) { }
MaterialProperty::MaterialProperty(float flt) : type(MaterialPropertyType_float), value(new MaterialPropertyValue(flt)) { }
MaterialProperty::MaterialProperty(const Texture& texture) : type(MaterialPropertyType_texture), value(new MaterialPropertyValue(texture)) { }

Material::Material()
	: drawOrder(MaterialDrawOrder_Less),
	drawType(MaterialDrawType_Triangles)
{ }

void Material::setProperty(const std::string& key, const MaterialProperty& property)
{
	auto iter = properties.find(key);

	if (iter != properties.end()) {
		iter->second.type = property.type;
		iter->second.value = std::make_unique<MaterialProperty::MaterialPropertyValue>(*property.value);
	} else {
		MaterialProperty copyProp;
		copyProp.propertyId = nextId++;
		copyProp.type = property.type;
		copyProp.value = std::make_unique<MaterialProperty::MaterialPropertyValue>(*property.value);
		properties.insert(std::make_pair(key, copyProp));
	}
}

void Material::setTextures(const std::vector<Texture>& textures)
{
	for (unsigned int i = 0; i < textures.size(); i++) {
		MaterialProperty textureProperty(textures[i]);
		std::string key;
		if (textures[i].impl->type == TextureType_diffuse) {
			key = "texture_diffuse";
		} else if (textures[i].impl->type == TextureType_specular) {
			key = "texture_specular";
		} else if (textures[i].impl->type == TextureType_cubemap) {
			key = "cubemap";
			// not all cubemaps are skyboxes, but they are for now!
			this->drawOrder = MaterialDrawOrder_LEqual;
		}
		this->setProperty(key, textureProperty);
	}
}

void Material::apply(const Shader& shader) const
{
	this->apply(*shader.impl);
}

void Material::apply(const ShaderImpl& shader) const
{
	unsigned int curTexture = 0;

	for (auto iter = properties.begin(); iter != properties.end(); ++iter) {
		const std::string& propertyName = iter->first;
		const MaterialProperty& property = iter->second;

		// Simple mapping - assume we'll never have more than 2^56 total material properties and 2^8
		// shaders and that shaders are assigned ids sequentially (foolish to assume on all implementations?)
		assert (property.propertyId < ((uint64_t)1 << 56) && shader.getID() < (1 << 8));
		uint64_t hash = property.propertyId << 8 | shader.getID();
		auto cacheIter = shaderUniformCache.find(hash);

		if (cacheIter == shaderUniformCache.end()) {
			// Miss - get the shader uniform and store it
			GLint shaderUniformId = shader.getUniformLocation(("material." + propertyName));
			auto insertIterPair = shaderUniformCache.insert(std::make_pair(hash, shaderUniformId));
			cacheIter = insertIterPair.first;
		}
		unsigned shaderUniformId = cacheIter->second;

		switch(property.type) {
		case MaterialPropertyType_vec3:
			glUniform3f(shaderUniformId, property.value->vec3.x, property.value->vec3.y, property.value->vec3.z);
			break;
		case MaterialPropertyType_vec4:
			glUniform4f(shaderUniformId, property.value->vec4.x, property.value->vec4.y, property.value->vec4.z, property.value->vec4.w);
			break;
		case MaterialPropertyType_texture:
			glActiveTexture(GL_TEXTURE0 + curTexture); // Activate proper texture unit before binding
			if (property.value->texture.type == TextureType_cubemap) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, property.value->texture.id);
			} else {
				glBindTexture(GL_TEXTURE_2D, property.value->texture.id);
			}
			glUniform1i(shaderUniformId, curTexture);
			curTexture++;
			break;
		case MaterialPropertyType_float:
			glUniform1f(shaderUniformId, property.value->flt);
			break;
		}
		glCheckError();
	}

	int depthFunc;
	if (drawOrder == MaterialDrawOrder_Less) {
		depthFunc = GL_LESS;
	} else if (drawOrder == MaterialDrawOrder_LEqual) {
		depthFunc = GL_LEQUAL;
	}
	glDepthFunc(depthFunc);
}
