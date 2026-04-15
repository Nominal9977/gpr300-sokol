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
in vec4 vs_clip_space;

uniform Material material;
uniform vec3 camera_position;
uniform sampler2D reflection_map;
uniform sampler2D refraction_map;

// Projects clip-space to [0,1] UV. Flip Y for the reflected camera.
vec3 reflection(vec4 clip_space)
{
    vec2 uv = clip_space.xy / clip_space.w * 0.5 + 0.5;
    uv.y = 1.0 - uv.y;
    return texture(reflection_map, uv).rgb;
}

vec3 refraction(vec4 clip_space)
{
    vec2 uv = clip_space.xy / clip_space.w * 0.5 + 0.5;
    return texture(refraction_map, uv).rgb;
}

void main()
{
    vec3 reflect_color  = reflection(vs_clip_space);
    vec3 refract_color  = refraction(vs_clip_space);

    vec3  view_dir = normalize(camera_position - vs_position);
    float fresnel  = pow(1.0 - max(dot(normalize(vs_normal), view_dir), 0.0), 2.0);

    vec3 albedo = mix(refract_color, reflect_color, fresnel);

    frag_position = vec4(vs_position.xyz, 1.0);
    frag_normal   = vec4(normalize(vs_normal), 1.0);
    frag_albedo   = vec4(albedo, 1.0);
    frag_material = vec4(material.ambient, material.diffuse, material.specular, material.shininess);
}
