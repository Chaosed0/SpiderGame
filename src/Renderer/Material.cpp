
#include "Material.h"
#include "RenderUtil.h"

uint64_t Material::nextId = 0;
std::unordered_map<uint64_t, GLuint> Material::shaderUniformCache;

Material::Material()
	: drawOrder(GL_LESS),
	drawType(GL_TRIANGLES)
{ }

MaterialProperty Material::getProperty(const std::string& key)
{
	auto iter = properties.find(key);
	if (iter != properties.end()) {
		return iter->second;
	}
	MaterialProperty invalid;
	invalid.type = MaterialPropertyType_invalid;
	return invalid;
}

void Material::setProperty(const std::string& key, MaterialProperty property)
{
	auto iter = properties.find(key);

	if (iter != properties.end()) {
		property.propertyId = iter->second.propertyId;
		iter->second = property;
	} else {
		property.propertyId = nextId++;
		properties.emplace(key, property);
	}
}

void Material::apply(const Shader& shader) const
{
	unsigned int curTexture = 0;

	for (auto iter = properties.begin(); iter != properties.end(); ++iter) {
		const std::string& propertyName = iter->first;
		const MaterialProperty& property = iter->second;

		// Simple hash - assume we'll never have more than 2^56 total material properties and 2^8
		// shaders and that shaders are assigned ids sequentially (foolish to assume on all implementations?)
		assert (property.propertyId < ((uint64_t)1 << 56) && shader.getID() < (1 << 8));
		uint64_t hash = property.propertyId << 8 | shader.getID();
		auto cacheIter = shaderUniformCache.find(hash);

		if (cacheIter == shaderUniformCache.end()) {
			// Miss - get the shader uniform and store it
			GLuint shaderUniformId = shader.getUniformLocation(("material." + propertyName).c_str());
			auto insertIterPair = shaderUniformCache.insert(std::make_pair(hash, shaderUniformId));
			cacheIter = insertIterPair.first;
		}
		GLuint shaderUniformId = cacheIter->second;

		switch(property.type) {
		case MaterialPropertyType_vec3:
			glUniform3f(shaderUniformId, property.value.vec3.x, property.value.vec3.y, property.value.vec3.z);
			break;
		case MaterialPropertyType_vec4:
			glUniform4f(shaderUniformId, property.value.vec4.x, property.value.vec4.y, property.value.vec4.z, property.value.vec4.w);
			break;
		case MaterialPropertyType_texture:
			glActiveTexture(GL_TEXTURE0 + curTexture); // Activate proper texture unit before binding
			if (property.value.texture.type == TextureType_cubemap) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, property.value.texture.id);
			} else {
				glBindTexture(GL_TEXTURE_2D, property.value.texture.id);
			}
			glUniform1i(shaderUniformId, curTexture);
			curTexture++;
			break;
		case MaterialPropertyType_float:
			glUniform1f(shaderUniformId, property.value.flt);
			break;
		}
		glCheckError();
	}

	glDepthFunc(drawOrder);
}
