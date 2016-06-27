#version 330 core

struct Material {
	sampler2D texture_diffuse;
};

in vec2 textureCoord;
in vec3 tintColor;

out vec4 color;

uniform Material material;

void main() {
    // Output color = color of the texture at the specified UV
	vec4 textureColor = texture( material.texture_diffuse, textureCoord ).rgba;
    color = vec4(tintColor * textureColor.rgb, textureColor.a);
}