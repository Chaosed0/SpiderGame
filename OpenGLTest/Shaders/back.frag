#version 330 core
out vec4 color;

uniform vec3 backColor;

void main()
{    
    color = vec4(backColor, 0.5f);
}  
