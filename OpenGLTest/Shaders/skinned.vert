#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 textureCoord_in;
layout (location = 3) in ivec4 boneIDs;
layout (location = 4) in vec4 weights;

const int MAX_BONES = 100;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 bones[MAX_BONES];

out vec3 fragPos;
out vec3 normal;
out vec2 textureCoord;

void main()
{
	mat4 bone_transform = bones[BoneIDs[0]] * weights[0];
	bone_transform += bones[BoneIDs[1]] * weights[1];
	bone_transform += bones[BoneIDs[2]] * weights[2];
	bone_transform += bones[BoneIDs[3]] * weights[3];

	vec4 pos_anim = bone_transform * vec4(position, 1.0f);
	vec4 normal_anim = bone_transform * vec4(normal_in, 0.0f)

    gl_Position = projection * view * model * pos_anim;
	fragPos = vec3(view * model * vec4(position, 1.0f));
	normal = mat3(transpose(inverse(view * model))) * normal_anim;
	textureCoord = textureCoord_in;
} 