#version 330 core

in vec3 positionOut;
in vec3 colorOut;
in vec2 texCoordOut;
in vec3 normalOut;

out vec4 fragColor;

uniform sampler2D textures0;
uniform sampler2D textures1;
uniform sampler2D textures2;
uniform sampler2D textures3;
uniform sampler2D textures4;
uniform sampler2D textures5;
uniform sampler2D textures6;
uniform sampler2D textures7;

uniform int numTextures;
uniform vec4 lightColor;
uniform vec3 camPos;

uniform struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	vec3 emission;
	float shininess;
} material; 

struct DirectionalSource {
	float intensity;
	vec3 direction;
};

uniform DirectionalSource dSource;

void direcLight(inout float diffSum, inout float specSum)
{
    // diffuse lighting
    vec3 normal = normalize(normalOut);
    vec3 lightDirection = normalize(dSource.direction);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

    // specular lighting
    float specularLight = 0.50f;
    vec3 viewDirection = normalize(camPos - positionOut);
    vec3 reflectionDirection = reflect(-lightDirection, normal);
    float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
    float specular = specAmount * specularLight;

	diffSum += diffuse * dSource.intensity;
    specSum += specular * dSource.intensity;
}

void main() {
	float diffSum = 0.0;
	float specSum = 0.0;
	direcLight(diffSum, specSum);	

	fragColor = (texture(textures0, texCoordOut) * (material.diffuse.x * diffSum + material.ambient.x) + texture(textures1, texCoordOut).r * material.specular.x * specSum) * lightColor;
}