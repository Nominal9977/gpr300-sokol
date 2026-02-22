#version 420 core

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view_proj;

out vec3 vs_position;
out vec3 vs_normal;
out vec2 vs_texcoord;

void main()
{
    vec4 worldPos = model * vec4(aPos, 1.0);
    vs_position = worldPos.xyz;
    vs_normal   = mat3(transpose(inverse(model))) * aNormal;
    vs_texcoord = aUV;

    gl_Position = view_proj * worldPos;   
}