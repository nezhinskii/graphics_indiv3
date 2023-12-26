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

struct PointSource {
	float intensity;
	vec3 pos;
};

struct SpotlightSource {
	float intensity;
	vec3 pos;
	vec3 direction;
	float cone;
	float outerCone;
};

struct DirectionalSource {
	float intensity;
	vec3 direction;
};

uniform PointSource pSource;
uniform SpotlightSource sSource;
uniform DirectionalSource dSource;

void pointLight(inout float diffSum, inout float specSum)
{
	// used in two variables so I calculate it here to not have to do it twice
	vec3 lightVec = pSource.pos - positionOut;

	// intensity of light with respect to distance
	float dist = length(lightVec);
	float a = 2.0;
	float b = 0.7;
	float inten = pSource.intensity / (a * dist * dist + b * dist + 1.0f);

	// diffuse lighting
	vec3 normal = normalize(normalOut);
	vec3 lightDirection = normalize(lightVec);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	if (diffuse > 0.95) diffuse = 1.0;
    else if (diffuse > 0.5) diffuse = 0.7;
    else if (diffuse > 0.25) diffuse = 0.3;
    else diffuse = 0.0;

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - positionOut);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), material.shininess);
	float specular = specAmount * specularLight;

	diffSum += diffuse * inten;
	specSum += specular * inten;
}

void spotLight(inout float diffSum, inout float specSum)
{
	// diffuse lighting
	vec3 normal = normalize(normalOut);
	vec3 lightDirection = normalize(sSource.pos - positionOut);
	float diffuse = max(dot(normal, lightDirection), 0.0f);

	if (diffuse > 0.95) diffuse = 1.0;
    else if (diffuse > 0.5) diffuse = 0.7;
    else if (diffuse > 0.25) diffuse = 0.3;
    else diffuse = 0.0;

	// specular lighting
	float specularLight = 0.50f;
	vec3 viewDirection = normalize(camPos - positionOut);
	vec3 reflectionDirection = reflect(-lightDirection, normal);
	float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 16);
	float specular = specAmount * specularLight;

	// calculates the intensity of the positionOut based on its angle to the center of the light cone
	float angle = dot(sSource.direction, -lightDirection);
	float inten = clamp((angle - sSource.outerCone) / (sSource.cone - sSource.outerCone), 0.0f, 1.0f);

	diffSum += diffuse * inten * sSource.intensity;
	specSum += specular * inten * sSource.intensity;
}

void direcLight(inout float diffSum, inout float specSum)
{
    // diffuse lighting
    vec3 normal = normalize(normalOut);
    vec3 lightDirection = normalize(dSource.direction);
    float diffuse = max(dot(normal, lightDirection), 0.0f);

	if (diffuse > 0.95) diffuse = 1.0;
    else if (diffuse > 0.5) diffuse = 0.7;
    else if (diffuse > 0.25) diffuse = 0.3;
    else diffuse = 0.0;

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
	float ambient = 0.20f;
	float diffSum = 0.0;
	float specSum = 0.0;
	pointLight(diffSum, specSum);
	spotLight(diffSum, specSum);
	direcLight(diffSum, specSum);	

	fragColor = (texture(textures0, texCoordOut) * (material.diffuse.x * diffSum + material.ambient.x) + texture(textures1, texCoordOut).r * material.specular.x * specSum) * lightColor;
}