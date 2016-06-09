#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D fontAtlas;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(fontAtlas, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}  