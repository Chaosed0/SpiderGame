#version 330 core
in vec3 textureCoords;
out vec4 color;

struct Material {
	samplerCube cubemap;
};
uniform Material material;

void main()
{    
    color = texture(material.cubemap, textureCoords);
}
  