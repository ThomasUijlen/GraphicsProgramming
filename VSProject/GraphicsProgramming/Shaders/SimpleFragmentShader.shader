#version 330 core

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform sampler2D albedoTexture;
uniform vec3 lightDirection;
uniform vec3 ambientLightColor;

void main()
{
    // Ambient
    vec3 ambient = ambientLightColor * vec3(texture(albedoTexture, UV));

    // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(norm, lightDir), 0.0);
    diff = floor(diff / 0.2) * 0.2;
    vec3 diffuse = diff * vec3(texture(albedoTexture, UV));

    // Combining all
    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0);
}
