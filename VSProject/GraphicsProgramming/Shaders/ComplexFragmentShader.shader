#version 330 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in mat3 TBN;

uniform sampler2D albedoTexture;
uniform sampler2D normalTexture;
uniform sampler2D specularTexture;
uniform vec3 ambientLightColor;
uniform vec3 lightDirection;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    // Obtain normal from normal map in range [0,1]
    vec3 normal = texture(normalTexture, TexCoords).rgb;
    // Transform normal vector to range [-1,1]
    normal = normalize(normal * 2.0 - 1.0);
    // Transform normal vector to world space
    normal = normalize(TBN * normal);

    // Obtain specular intensity from specular map
    float specularIntensity = texture(specularTexture, TexCoords).r;

    // Obtain diffuse color from albedo map
    vec3 albedo = texture(albedoTexture, TexCoords).rgb;

    // Ambient lighting
    vec3 ambient = ambientLightColor * albedo;

    // Diffuse lighting
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * albedo;

    // Specular lighting
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), specularIntensity);
    vec3 specular = spec * ambientLightColor;

    // Combine results
    vec3 result = ambient + diffuse + specular;

    FragColor = vec4(result, 1.0);
}
