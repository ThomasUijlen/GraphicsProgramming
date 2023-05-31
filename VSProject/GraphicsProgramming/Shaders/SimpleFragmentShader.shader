#version 330 core

in vec3 outColor;
in vec2 outUV;
in mat3 TBN;
in vec3 worldPosition;

out vec4 FragColor;

uniform sampler2D boxTexture;
uniform sampler2D boxNormal;
uniform vec3 lightPosition;
uniform vec3 ambientLightColor;
uniform vec3 cameraPosition;
uniform float shininess;

void main()
{
    vec3 normal = texture(boxNormal, outUV).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = TBN * normal;

    vec3 lightDirection = normalize(lightPosition - worldPosition);
    vec3 viewDirection = normalize(cameraPosition - worldPosition);

    // Ambient
    vec3 ambient = ambientLightColor * vec3(texture(boxTexture, outUV));

    // Diffuse
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diff * vec3(texture(boxTexture, outUV));

    // Specular
    vec3 reflectDir = reflect(-lightDirection, normal);  
    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), shininess);
    vec3 specular = spec * vec3(1.0, 1.0, 1.0); // assuming white specular color for simplicity

    vec3 result = (ambient + diffuse + specular) * outColor;

    FragColor = vec4(result, 1.0);
}
