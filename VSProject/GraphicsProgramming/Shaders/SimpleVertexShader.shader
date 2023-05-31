#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vUV;
layout(location = 3) in vec3 normal;
layout(location = 4) in vec3 tangent;
layout(location = 5) in vec3 bitangent;

out vec3 outColor;
out vec2 outUV;
out mat3 TBN;
out vec3 worldPosition;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * world * vec4(position, 1.0);
    outColor = vertexColor;
    outUV = vUV;

    vec3 T = normalize(mat3(world) * tangent);
    vec3 B = normalize(mat3(world) * bitangent);
    vec3 N = normalize(mat3(world) * normal);

    // calculate TBN matrix
    TBN = mat3(T, B, N);

    worldPosition = mat3(world) * position;
}
