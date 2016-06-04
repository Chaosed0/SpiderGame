#pragma once

#include <glm/glm.hpp>

#include <Windows.h>
#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/GLU.h>

#include <vector>

#include "Shader.h"
#include "Texture.h"
#include "Transform.h"
#include "Material.h"

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoords;
};

struct Mesh
{
	GLuint VAO, VBO, EBO;
	GLuint nVertices, nIndices;
	Material material;
	Mesh();
	Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<Texture> textures);
};

struct Model
{
	std::vector<Mesh> meshes;
	unsigned int id;
	Model() {}
	Model(std::vector<Mesh> meshes) : meshes(meshes) {}
};