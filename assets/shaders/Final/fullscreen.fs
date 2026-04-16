#version 410

out vec4 FragColor;
in vec2 vUV;

uniform sampler2D screen;
uniform sampler2D g_albedo;
uniform sampler2D g_normal;
uniform vec3 ambient_color;
uniform float ambient_strength;
uniform vec3 sky_color;
uniform vec3 sun_dir;
uniform vec3 sun_color;
uniform float sun_intensity;

void main()
{
    vec4 albedo_sample = texture(g_albedo, vUV);

    //show the sky
    if (albedo_sample.a == 0.0)
    {
        FragColor = vec4(sky_color, 1.0);
        return;
    }

    vec3 color  = texture(screen, vUV).rgb;
    vec3 albedo = albedo_sample.rgb;

    // Water pixels are pre-lit 
    if (albedo_sample.a < 0.75)
    {
        FragColor = vec4(albedo, 1.0);
        return;
    }

    // Directional sun
    vec3 normal    = normalize(texture(g_normal, vUV).xyz);
    float sun_diff = max(dot(normal, sun_dir), 0.0);
    vec3  sun_contrib = sun_diff * sun_color * sun_intensity * albedo;

    vec3 ambient = ambient_color * albedo * ambient_strength;
    FragColor = vec4(color + ambient + sun_contrib, 1.0);
}