#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 textureCoord_in;
layout (location = 3) in vec3 tintColor_in;
layout (location = 4) in ivec4 boneIDs;
layout (location = 5) in vec4 weights;

const int MAX_BONES = 100;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bones[MAX_BONES];

out vec3 fragPos;
out vec3 normal;
out vec2 textureCoord;
out vec3 tintColor;

void main()
{
	mat4 bone_transform = bones[boneIDs[0]] * weights[0];
	bone_transform += bones[boneIDs[1]] * weights[1];
	bone_transform += bones[boneIDs[2]] * weights[2];
	bone_transform += bones[boneIDs[3]] * weights[3];

	vec4 pos_anim = bone_transform * vec4(position, 1.0f);
	vec4 normal_anim = bone_transform * vec4(normal_in, 0.0f);

    gl_Position = projection * view * model * pos_anim;
	fragPos = vec3(view * model * pos_anim);
	normal = (transpose(inverse(view * model)) * normal_anim).xyz;
	tintColor = tintColor_in;
	textureCoord = textureCoord_in;
} 