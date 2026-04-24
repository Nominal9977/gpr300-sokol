#version 410

layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

uniform mat4 model;
uniform mat4 view_proj;
uniform mat4 light_view_proj;

out vec3 vs_position;
out vec3 vs_normal;
//out vec2 vs_texcoord;
out vec4 vs_light_proj_pos;

void main()
{
    vs_position = vec3(model * vec4(aPos, 1.0));

    vs_normal = aNormal;
    //vs_texcoord = aUV;

    vs_light_proj_pos = light_view_proj * vec4(vs_position, 1.0);
    gl_Position = view_proj* vec4(vs_position, 1.0);

}