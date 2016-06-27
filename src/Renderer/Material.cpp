
#include "Material.h"
#include "RenderUtil.h"

Material::Material()
{
	drawOrder = GL_LESS;
}

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
		iter->second = property;
	} else {
		properties.emplace(key, property);
	}
}

void Material::apply(const Shader& shader) const
{
	unsigned int curTexture = 0;

	for (auto iter = properties.begin(); iter != properties.end(); ++iter) {
		const MaterialProperty& property = iter->second;
		GLuint propertyId = shader.getUniformLocation(("material." + iter->first).c_str());
		switch(property.type) {
		case MaterialPropertyType_vec3:
			glUniform3f(propertyId, property.value.vec3.x, property.value.vec3.y, property.value.vec3.z);
			break;
		case MaterialPropertyType_vec4:
			glUniform4f(propertyId, property.value.vec4.x, property.value.vec4.y, property.value.vec4.z, property.value.vec4.w);
			break;
		case MaterialPropertyType_texture:
			glActiveTexture(GL_TEXTURE0 + curTexture); // Activate proper texture unit before binding
			if (property.value.texture.type == TextureType_cubemap) {
				glBindTexture(GL_TEXTURE_CUBE_MAP, property.value.texture.id);
			} else {
				glBindTexture(GL_TEXTURE_2D, property.value.texture.id);
			}
			glUniform1i(propertyId, curTexture);
			curTexture++;
			break;
		case MaterialPropertyType_float:
			glUniform1f(propertyId, property.value.flt);
			break;
		}
		glCheckError();
	}

	glDepthFunc(drawOrder);
}
