#version 410


layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_texcoord;


uniform mat4 view_proj;
uniform mat4 ref_view_proj; 
uniform mat4 model;


out vec3 vs_position;
out vec3 vs_normal;
out vec2 vs_texcoord;
out vec4 vs_clip_reflect; 

void main()
{
    vec4 world_position = model * vec4(in_position, 1.0);           //world space
    vec3 world_normal = transpose(inverse(mat3(model))) * in_normal; //scale normals

    vs_position    = world_position.xyz;
    vs_normal      = world_normal;
    vs_texcoord    = in_texcoord;
    vs_clip_reflect = ref_view_proj * world_position; // position from reflection camera

    gl_Position = view_proj * world_position; // project to screen
}
