#version 410

precision mediump float;

struct Material {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

// g-buffer attachments
layout(location = 0) out vec4 frag_position;
layout(location = 1) out vec4 frag_normal;
layout(location = 2) out vec4 frag_albedo;
layout(location = 3) out vec4 frag_material;

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform Material material;

// uniform sampler2D reflection_map;
// uniform sampler2D refraction_map;
// uniform sampler2D dudv_map;
// uniform float wave_offset;

void main()
{
    // Need to distort UVs with dudv map for wave effect
    vec2 uv = vs_texcoord;

    // need to blend reflection and refraction textures here
    vec3 water_color = vec3(0.05, 0.35, 0.75);

    frag_position = vec4(vs_position.xyz, 1.0);
    frag_normal   = vec4(normalize(vs_normal), 1.0);
    frag_albedo   = vec4(water_color, 1.0);
    frag_material = vec4(material.ambient, material.diffuse, material.specular, material.shininess);
}
