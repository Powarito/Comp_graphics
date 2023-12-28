//FRAGMENT SHADER

#version 330 core

in vec2 TexCoords;

out vec4 FragColor;


struct Material {
    sampler2D texture_diffuse1;
    sampler2D texture_emissive1;
};

uniform Material material;
uniform bool useEmission;


void main()
{
    vec3 result = vec3(texture(material.texture_diffuse1, TexCoords));

    vec3 emission = vec3(0.0);
    if (useEmission)
        emission = vec3(texture(material.texture_emissive1, TexCoords));

    FragColor = vec4(result + emission, 1.0);
}