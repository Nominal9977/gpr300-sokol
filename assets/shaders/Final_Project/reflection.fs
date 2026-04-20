#version 410

precision mediump float;

layout(location = 0) out vec4 FragColor;

in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform sampler2D albedo_tex;
uniform bool has_texture;
uniform vec3 sun_dir;
uniform vec3 sun_color;
uniform float sun_intensity;

void main()
{
    vec3 normal = normalize(vs_normal);

    // If it has a texture use it
    vec3 base_color = has_texture ? texture(albedo_tex, vs_texcoord).rgb : normal * 0.5 + 0.5;

    float diff    = max(dot(normal, sun_dir), 0.0); // diffuse angle to sun
    vec3  ambient = 0.25 * base_color; //ambient
    vec3  diffuse = diff * sun_color * sun_intensity * base_color; //diffuse

    FragColor = vec4(ambient + diffuse, 1.0); //ambient and diffuse
}
