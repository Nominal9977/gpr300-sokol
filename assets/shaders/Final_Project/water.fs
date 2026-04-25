#version 410

precision mediump float;

struct Material {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
};

layout(location = 0) out vec4 frag_position;
layout(location = 1) out vec4 frag_normal;
layout(location = 2) out vec4 frag_albedo;
layout(location = 3) out vec4 frag_material;

in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;
in vec4 vs_clip_reflect;

uniform Material material;
uniform vec3 camera_position;
uniform vec2 screen_size;
uniform sampler2D reflection_map;
uniform sampler2D refraction_map;
uniform sampler2D refraction_depth;
uniform sampler2D wave_tex;
uniform sampler2D wave_warp;
uniform vec3  fog_color;
uniform float fog_max_depth;
uniform float fresnel_F0;
uniform float near_plane;
uniform float far_plane;
uniform float time;
uniform vec3  sun_dir;
uniform vec3  sun_color;
uniform float sun_intensity;
uniform float refraction_strength;

float linearize_depth(float d)
{
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - (d * 2.0 - 1.0) * (far_plane - near_plane));
}

float edge_fade(vec2 uv, float margin)
{
    vec2 d = smoothstep(0.0, margin, min(uv, 1.0 - uv));
    return min(d.x, d.y);
}

// Returns 0 = shallow/on terrain, 1 = deep or no terrain below
float waterDepth(vec2 uv)
{
    float raw = texture(refraction_depth, uv).r;
    if (raw >= 0.9999)
        return 1.0;

    float terrain_d = linearize_depth(raw);
    float water_d = linearize_depth(gl_FragCoord.z);
    float diff = max(terrain_d - water_d, 0.0);
    return clamp(diff / fog_max_depth, 0.0, 1.0);
}

void main()
{
    vec2 uv1 = vs_texcoord + vec2( time * 0.02,  time * 0.01);
    vec2 uv2 = vs_texcoord + vec2(-time * 0.015, time * 0.025);

    vec2 distort = ((texture(wave_warp, uv1).rg * 2.0 - 1.0) + (texture(wave_warp, uv2).rg * 2.0 - 1.0)) * 0.03 * refraction_strength;
    vec3 surface = mix(texture(wave_tex, uv1).rgb, texture(wave_tex, uv2).rgb, 0.5);

    vec2 screen_uv = gl_FragCoord.xy / screen_size;
    float fade = edge_fade(screen_uv, 0.05);
    vec2 distort_s = distort * fade;

    vec2 reflect_ndc = vs_clip_reflect.xy / vs_clip_reflect.w;
    vec2 reflect_uv = clamp(reflect_ndc * 0.5 + 0.5 + distort_s, 0.001, 0.999);
    vec2 refract_uv = clamp(screen_uv + distort_s, 0.001, 0.999);

    float depth = waterDepth(screen_uv);

    float dist_raw = texture(refraction_depth, refract_uv).r;
    float water_d = linearize_depth(gl_FragCoord.z);
    bool  valid_refuv = (dist_raw < 0.9999) && (linearize_depth(dist_raw) > water_d);
    vec2  safe_refract = valid_refuv ? refract_uv : screen_uv;

    //blend toward fog color as depth increases
    vec3 refract_color = texture(refraction_map, safe_refract).rgb;
    vec3 refract_dist = mix(refract_color, fog_color, depth);

    // Apply water tint
    vec3 water_tint = vec3(0.0, 0.2, 0.4);
    refract_dist = mix(refract_dist, water_tint, 0.15);

    vec3 reflect_dist = texture(reflection_map, reflect_uv).rgb;

    vec3  view_dir = normalize(camera_position - vs_position);
    float cos_theta = max(dot(normalize(vs_normal), view_dir), 0.0);
    float fresnel = fresnel_F0 + (1.0 - fresnel_F0) * pow(1.0 - cos_theta, 3.0);
    float eff_fresnel = fresnel * (1.0 - depth * 0.9);

    vec3 water = mix(refract_dist, reflect_dist, eff_fresnel);

    // Sun specular
    vec3 half_dir = normalize(sun_dir + view_dir);
    float spec = pow(max(dot(normalize(vs_normal), half_dir), 0.0), 128.0);
    vec3 sun_spec = spec * sun_color * sun_intensity * 0.9;

    vec3 albedo = mix(water, surface, 0.15) + sun_spec;

    frag_position = vec4(vs_position.xyz, 1.0);
    frag_normal = vec4(normalize(vs_normal), 1.0);
    frag_albedo = vec4(albedo, 0.5);
    frag_material = vec4(material.ambient, material.diffuse, material.specular, material.shininess);
}
