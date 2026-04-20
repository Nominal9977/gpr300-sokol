#version 410
// final composite pass - combines lighting, albedo, ambient, and sun onto the screen

out vec4 FragColor;
in vec2 vUV;

uniform sampler2D screen;    // from blinnphong pass
uniform sampler2D g_albedo;  // albedo 
uniform sampler2D g_normal;  // normals 
uniform vec3 ambient_color;
uniform float ambient_strength;
uniform vec3 sky_color;
uniform vec3 sun_dir;
uniform vec3 sun_color;
uniform float sun_intensity;

void main()
{
    vec4 albedo_sample = texture(g_albedo, vUV); //albedo at this pixel

    // show sky where nothing was drawn
    if (albedo_sample.a == 0.0)
    {
        FragColor = vec4(sky_color, 1.0);
        return;
    }

    vec3 color  = texture(screen, vUV).rgb; // grab point light result
    vec3 albedo = albedo_sample.rgb;

    // water pixels are pre-lit, output directly
    if (albedo_sample.a < 0.75)
    {
        FragColor = vec4(albedo, 1.0);
        return;
    }

    // directional sun contribution
    vec3 normal    = normalize(texture(g_normal, vUV).xyz); // get surface normal
    float sun_diff = max(dot(normal, sun_dir), 0.0);        // diffuse angle to sun
    vec3  sun_contrib = sun_diff * sun_color * sun_intensity * albedo; // sun light on surface

    vec3 ambient = ambient_color * albedo * ambient_strength; // flat ambient fill
    FragColor = vec4(color + ambient + sun_contrib, 1.0); // combine all lighting
}
