#version 330 core

struct Material {
	sampler2D texture_diffuse1;
};

in vec2 textureCoord;

out vec4 color;

uniform Material material;

void main() {
    // Output color = color of the texture at the specified UV
    color = texture( material.texture_diffuse1, textureCoord ).rgba;
}