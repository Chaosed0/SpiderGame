#version 330 core
in vec2 textureCoord;
out vec4 color;

struct Material {
	vec4 textColor;
	// should be the font atlas
	sampler2D texture_diffuse;
};
uniform Material material;

void main()
{
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(material.texture_diffuse, textureCoord).r);
    color = material.textColor * sampled;
}