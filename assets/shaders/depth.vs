#version 410

layout(location=0) in vec3 in_postion;


uniform mat4 model;
uniform mat4 light_view_projection;

void main()
{
    vec4 worldPos = model* vec4(in_postion, 1.0);
 gl_Position = light_view_projection * worldPos;
}