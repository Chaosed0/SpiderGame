#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color_in;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 fragColor;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
	fragColor = vec4(color_in, 1.0f);
} 
