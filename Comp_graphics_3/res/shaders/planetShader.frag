//FRAGMENT SHADER

#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;


struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_specular1;
    sampler2D texture_emissive1;
    float     shininess;
};


struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
 
    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

uniform Material material;
uniform vec3 viewPos;

uniform bool useEmission;

#define MAX_DIR_LIGHTS 5
#define MAX_POINT_LIGHTS 100
#define MAX_SPOT_LIGHTS 20

uniform int NR_DIR_LIGHTS;
uniform int NR_POINT_LIGHTS;
uniform int NR_SPOT_LIGHTS;

uniform DirLight dirLights[MAX_DIR_LIGHTS];
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform SpotLight spotLights[MAX_SPOT_LIGHTS];

// Function prototypes
vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);

 
void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

    for(int i = 0; i < NR_DIR_LIGHTS; ++i)
        result += CalcDirLight(dirLights[i], norm, viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; ++i)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);

    for(int i = 0; i < NR_SPOT_LIGHTS; ++i)
        result += CalcSpotLight(spotLights[i], norm, FragPos, viewDir);

    vec3 emission = vec3(0.0);
    if (useEmission)
        emission = vec3(texture(material.texture_emissive1, TexCoords));

    FragColor = vec4(result + emission, 1.0);
}


vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
 
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
 
    // Specular component
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
 
    // Combining results
    //vec3 texture_diffuse_combined = vec3(texture(material.texture_diffuse1, TexCoords)) +
    //                                vec3(texture(material.texture_diffuse2, TexCoords)) +
    //                                vec3(texture(material.texture_diffuse3, TexCoords));
    //
    //vec3 texture_specular_combined = vec3(texture(material.texture_specular1, TexCoords)) +
    //                                 vec3(texture(material.texture_specular2, TexCoords));


    //vec3 ambient = light.ambient * texture_diffuse_combined;
    //vec3 diffuse = light.diffuse * diff * texture_diffuse_combined;
    //vec3 specular = light.specular * spec * texture_specular_combined;

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));
    
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - fragPos);
 
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
 
    // Specular component
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
 
    // Attenuation
    float distance = length(light.position - fragPos);
    float attenuation = light.constant / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    //float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));

    // Combining results
    //vec3 texture_diffuse_combined = vec3(texture(material.texture_diffuse1, TexCoords)) +
    //                                vec3(texture(material.texture_diffuse2, TexCoords)) +
    //                                vec3(texture(material.texture_diffuse3, TexCoords));
    //
    //vec3 texture_specular_combined = vec3(texture(material.texture_specular1, TexCoords)) +
    //                                 vec3(texture(material.texture_specular2, TexCoords));


    //vec3 ambient = light.ambient * texture_diffuse_combined;
    //vec3 diffuse = light.diffuse * diff * texture_diffuse_combined;
    //vec3 specular = light.specular * spec * texture_specular_combined;

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));

    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - FragPos);

    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular component
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    
    // Spotlight
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = (light.cutOff - light.outerCutOff);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Attenuation
    float distance = length(light.position - FragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Combining results
    //vec3 texture_diffuse_combined = vec3(texture(material.texture_diffuse1, TexCoords)) +
    //                                vec3(texture(material.texture_diffuse2, TexCoords)) +
    //                                vec3(texture(material.texture_diffuse3, TexCoords));
    //
    //vec3 texture_specular_combined = vec3(texture(material.texture_specular1, TexCoords)) +
    //                                 vec3(texture(material.texture_specular2, TexCoords));


    //vec3 ambient = light.ambient * texture_diffuse_combined;
    //vec3 diffuse = light.diffuse * diff * texture_diffuse_combined;
    //vec3 specular = light.specular * spec * texture_specular_combined;

    vec3 ambient = light.ambient * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.texture_specular1, TexCoords));

    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;

    return (ambient + diffuse + specular);
}