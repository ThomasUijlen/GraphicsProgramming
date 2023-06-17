#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

out vec2 UV;
out vec3 Normal;
out vec3 FragPos;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(inPos, 1.0);
    FragPos = vec3(model * vec4(inPos, 1.0));
    UV = inUV;

    vec3 T = normalize(mat3(model) * inTangent);
    vec3 B = normalize(mat3(model) * inBitangent);
    Normal = normalize(mat3(model) * inNormal);

    TBN = mat3(T, B, Normal);
}
