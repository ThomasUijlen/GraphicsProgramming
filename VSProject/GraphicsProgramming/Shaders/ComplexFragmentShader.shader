#version 330 core

in vec2 UV;
in vec3 FragPos;
in vec3 Normal;
in mat3 TBN;

out vec4 FragColor;

uniform sampler2D albedoTexture;
uniform sampler2D normalMap;
uniform vec3 lightDirection;
uniform vec3 ambientLightColor;
uniform vec3 cameraPos;
uniform float shininess;

void main()
{
    // Obtain normal from normal map
    vec3 normalFromMap = texture(normalMap, UV).rgb;
    normalFromMap = normalize(normalFromMap * 2.0 - 1.0);  // this maps the color from [0:1] to [-1:1]
    vec3 NormalMap = normalize(TBN * normalFromMap);

    // Ambient
    vec3 ambient = ambientLightColor * vec3(texture(albedoTexture, UV));

    // Diffuse 
    vec3 lightDir = normalize(-lightDirection);
    float diff = max(dot(NormalMap, lightDir), 0.0);
    vec3 diffuse = diff * vec3(texture(albedoTexture, UV));

    // Specular
    vec3 viewDir = normalize(cameraPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, NormalMap);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = vec3(0.3) * spec;  // Assuming the specular color is white

    // Combining all
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, 1.0);
}
