#version 410

precision mediump float;

layout(location = 0) out vec4 FragColor;

in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

void main()
{
    vec3 color = vs_normal * 0.5 + 0.5;
    FragColor = vec4(color, 1.0);
}