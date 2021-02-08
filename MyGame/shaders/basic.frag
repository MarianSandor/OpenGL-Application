#version 410 core

in vec4 fragPosLightSpace;

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//spotlight 
struct Spotlight {
    mat4 view;
    mat3 normalM;

    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 color;
};
#define NR_SPOT_LIGHTS 17
uniform Spotlight spotlight[NR_SPOT_LIGHTS];

struct Pointlight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 color;
};
#define NR_POINT_LIGHTS 16
uniform Pointlight pointlight[NR_POINT_LIGHTS];

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

uniform sampler2D shadowMap;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

float computeShadow()
{
    float shadow;
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	normalizedCoords = normalizedCoords * 0.5 + 0.5;

	if (normalizedCoords.z > 1.0f) {
		shadow = 0.0f;
	}
	else {
		float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
		float currentDepth = normalizedCoords.z;

		float bias = max(0.05f * (1.0f - dot(fNormal, lightDir)), 0.005f);

		shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	}

    return shadow;
}

vec3 computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;

    float shadow = computeShadow();

    return min((ambient + (1.0f - shadow)*diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0f - shadow) * specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
}

vec3 computeSpotLight(Spotlight light)
{
    vec4 fPosEye = light.view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(light.normalM * fNormal);
    vec3 lightPosEye = vec3(light.view * vec4(light.position, 1.0f));
    vec3 viewDir = normalize(-fPosEye.xyz);

    vec3 lightDirN = normalize(lightPosEye - vec3(fPosEye));

    // diffuse shading
    float diff = max(dot(normalEye, lightDirN), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    // attenuation
    float distToLight = length(lightPosEye - vec3(fPosEye));
    float attenuation = 1.0f / (light.constant + light.linear * distToLight + light.quadratic * (distToLight * distToLight));   
    // spotlight intensity
    float theta = dot(lightDirN, normalize(-vec3(light.view * vec4(light.direction, 1.0f)))); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
    // combine results
    vec3 ambient = light.color * texture(diffuseTexture, fTexCoords).rgb;
    vec3 diffuse = diff * light.color * texture(diffuseTexture, fTexCoords).rgb;
    vec3 specular = specCoeff * light.color * texture(specularTexture, fTexCoords).rgb;

    ambient *= attenuation * intensity;;
    diffuse *= attenuation * intensity;;
    specular *= attenuation * intensity;;

    return clamp((ambient + diffuse + specular), 0.0f, 1.0f); 
}

vec3 computePointLight(Pointlight light)
{
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 lightPosEye = vec3(view * vec4(light.position, 1.0f));
    vec3 viewDir = normalize(-fPosEye.xyz);

    vec3 lightDirN = normalize(lightPosEye - vec3(fPosEye));

    // diffuse shading
    float diff = max(dot(normalEye, lightDirN), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    // attenuation
    float distToLight = length(lightPosEye - vec3(fPosEye));
    float attenuation = 1.0f / (light.constant + light.linear * distToLight + light.quadratic * (distToLight * distToLight));    
    
    // combine results
    vec3 ambient = light.color * texture(diffuseTexture, fTexCoords).rgb;
    vec3 diffuse = diff * light.color * texture(diffuseTexture, fTexCoords).rgb;
    vec3 specular = specCoeff * light.color * texture(specularTexture, fTexCoords).rgb;

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    return clamp((ambient + diffuse + specular), 0.0f, 1.0f); 
}

void main() 
{
    vec3 color = computeDirLight();

    for (int i = 0; i < NR_POINT_LIGHTS; i++) {
        color += computePointLight(pointlight[i]);
    }

    for (int i = 0; i < NR_SPOT_LIGHTS; i++) {
        color += computeSpotLight(spotlight[i]);
    }

    fColor = vec4(color, 1.0f);
}
