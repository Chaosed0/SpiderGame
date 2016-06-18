#version 330 core
out vec4 color;

struct Material {
	vec4 color;
};

uniform Material material;

void main()
{    
    color = material.color;
}  
