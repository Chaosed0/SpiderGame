#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 textureCoord_in;
layout (location = 3) in vec3 tintColor_in;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 fragPos;
out vec3 normal;
out vec2 textureCoord;
out vec3 tintColor;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
	fragPos = vec3(view * model * vec4(position, 1.0f));
	normal = mat3(transpose(inverse(view * model))) * normal_in;
	tintColor = tintColor_in;
	textureCoord = textureCoord_in;
} 