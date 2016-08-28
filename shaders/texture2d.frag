#version 330 core
in vec2 textureCoord;
out vec4 color;

struct Material {
	sampler2D texture_diffuse;
};
uniform Material material;

void main()
{
    color = texture(material.texture_diffuse, textureCoord);
}