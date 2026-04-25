#version 410
// final composite pass - combines lighting, albedo, ambient, and sun onto the screen

out vec4 FragColor;
in vec2 vUV;

uniform sampler2D screen;       // from blinnphong pass
uniform sampler2D g_albedo;     // albedo
uniform sampler2D g_normal;     // normals
uniform sampler2D g_position;   // world-space positions
uniform sampler2D shadow_map;   // depth map from light's POV
uniform sampler2D g_material;   // r=ambient g=diffuse b=specular a=shininess
uniform mat4 light_view_proj;
uniform vec3 camera_position;
uniform vec3 ambient_color;
uniform float ambient_strength;
uniform vec3 sky_color;
uniform vec3 sun_dir;
uniform vec3 sun_color;
uniform float sun_intensity;

float ShadowFactor(vec3 world_pos, vec3 normal)
{
    vec4 light_space = light_view_proj * vec4(world_pos, 1.0);
    vec3 proj = light_space.xyz / light_space.w;
    proj = proj * 0.5 + 0.5;

    if (proj.z > 1.0)
        return 0.0;

    float bias = max(0.005 * (1.0 - dot(normal, sun_dir)), 0.001);
    float shadow = 0.0;
    vec2 texel = 1.0 / vec2(textureSize(shadow_map, 0));
    for (int x = -1; x <= 1; ++x)
        for (int y = -1; y <= 1; ++y)
        {
            float pcf_depth = texture(shadow_map, proj.xy + vec2(x, y) * texel).r;
            shadow += (proj.z - bias > pcf_depth) ? 1.0 : 0.0;
        }
    return shadow / 9.0;
}

void main()
{
    vec4 albedo_sample = texture(g_albedo, vUV);

    // show sky where nothing was drawn
    if (albedo_sample.a == 0.0)
    {
        FragColor = vec4(sky_color, 1.0);
        return;
    }

    vec3 color  = texture(screen, vUV).rgb;
    vec3 albedo = albedo_sample.rgb;

    // water pixels are pre-lit, output directly
    if (albedo_sample.a < 0.75)
    {
        FragColor = vec4(albedo, 1.0);
        return;
    }

    vec3  normal    = normalize(texture(g_normal, vUV).xyz);
    vec3  world_pos = texture(g_position, vUV).xyz;
    vec4  mat_data  = texture(g_material, vUV);

    float diffuse_k  = mat_data.g;
    float specular_k = mat_data.b;
    float shininess  = mat_data.a * 128.0 + 1.0;

    float shadow   = ShadowFactor(world_pos, normal);
    float sun_diff = max(dot(normal, sun_dir), 0.0);

    vec3 view_dir = normalize(camera_position - world_pos);
    vec3 half_dir = normalize(sun_dir + view_dir);
    float sun_spec = pow(max(dot(normal, half_dir), 0.0), shininess) * specular_k;

    vec3 sun_contrib = (sun_diff * diffuse_k * albedo + sun_spec * sun_color) * sun_intensity * (1.0 - shadow);
    vec3 ambient     = ambient_color * albedo * ambient_strength;
    FragColor = vec4(color + ambient + sun_contrib, 1.0);
}
