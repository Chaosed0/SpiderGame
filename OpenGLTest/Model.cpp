
#include "Model.h"

#include <sstream>
#include <string>

Mesh::Mesh()
{}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures)
{
	this->nVertices = vertices.size();
	this->nIndices = indices.size();

	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoords));

	glBindVertexArray(0);

	for (unsigned int i = 0; i < textures.size(); i++) {
		MaterialProperty textureProperty;
		if (textures[i].type == TextureType_diffuse) {
			textureProperty.key = "texture_diffuse";
			textureProperty.type = MaterialPropertyType_texture;
			textureProperty.value.texture = textures[i];
		} else if (textures[i].type == TextureType_specular) {
			textureProperty.key = "texture_specular";
			textureProperty.type = MaterialPropertyType_texture;
			textureProperty.value.texture = textures[i];
		} else if (textures[i].type == TextureType_cubemap) {
			textureProperty.key = "cubemap";
			textureProperty.type = MaterialPropertyType_texture;
			textureProperty.value.texture = textures[i];
			// not all cubemaps are skyboxes, but they are for now!
			material.drawOrder = GL_LEQUAL;
		}
		material.setProperty(textureProperty);
	}

	MaterialProperty shininessProperty;
	shininessProperty.key = "shininess";
	shininessProperty.type = MaterialPropertyType_float;
	shininessProperty.value.flt = 32.0f;
	material.setProperty(shininessProperty);
}
