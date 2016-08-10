#version 330 core

#define MAX_POINT_LIGHTS 64

struct Material {
    sampler2D texture_diffuse;
	sampler2D texture_specular;
    float shininess;
};

struct PointLight {
    vec3 position;

	float constant;
	float linear;
	float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

in vec3 fragPos;
in vec3 normal;
in vec2 textureCoord;
in vec3 tintColor;

out vec4 color;
  
uniform PointLight pointLight[MAX_POINT_LIGHTS];
uniform DirLight dirLight;
uniform Material material;
uniform mat4 view;
uniform int pointLightCount;

// Calculates the color when using a point light.
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos)
{
	vec3 viewDir = normalize(-fragPos);
	vec3 lightPos = vec3(view * vec4(light.position, 1));
    vec3 lightDir = normalize(lightPos - fragPos);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular
    vec3 reflectDir = reflect(-viewDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // Attenuation
    float distance    = length(lightPos - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    

    // Combine results
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse, textureCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse, textureCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular, textureCoord));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return (ambient + diffuse + specular);
} 

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 fragPos)
{
	vec3 viewDir = normalize(-fragPos);
    vec3 lightDir = normalize(mat3(view) * -light.direction);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0);

    // Specular
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    // Combine
    vec3 ambient  = light.ambient  * vec3(texture(material.texture_diffuse, textureCoord));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(material.texture_diffuse, textureCoord));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular, textureCoord));

    return ambient + diffuse + specular;
}  

void main()
{
	vec3 normal_n = normalize(normal);

	//vec3 result = CalcDirLight(dirLight, normal_n, fragPos);
	vec3 result = vec3(0.0f);
	for (int i = 0; i < min(MAX_POINT_LIGHTS, pointLightCount); i++) {
		result += CalcPointLight(pointLight[i], normal_n, fragPos);
	}

	result = result * tintColor;
	color = vec4(result, 1.0f);
}
