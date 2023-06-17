#version 330 core

in vec3 viewDir;

out vec4 FragColor;

uniform vec3 lightDirection;
uniform mat4 view;

void main()
{
    vec3 topColor = vec3(0.1, 0.3, 0.6); // dark blue
    vec3 bottomColor = vec3(0.9, 0.9, 1.0); // light blue
    vec3 sunColor = vec3(1.0, 1.0, 0.9); // slightly off-white

    float t = 0.5 * (viewDir.y + 1.0);
    vec3 skyColor = mix(bottomColor, topColor, t);

    vec3 viewLightDirection = vec3(view * vec4(-lightDirection, 0.0));

    float sunArea = 0.01;
    float sunFactor = smoothstep(1.0 - sunArea, 1.0, dot(normalize(viewDir), normalize(viewLightDirection)));

    vec3 finalColor = mix(skyColor, sunColor, sunFactor);

    FragColor = vec4(finalColor, 1.0);
}
