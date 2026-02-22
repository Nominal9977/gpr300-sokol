#version 420 core

out vec4 FragColor;

struct Light {
    vec3 color;
    vec3 postion;
};

struct Pallet {
    vec3 color1;
    vec3 color2;
};

struct Materail {
    vec3 ambeint;
    vec3 diffuse;
    vec3 specular;
    float shinniness;
};

in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform Light light;
uniform vec3 camera_position;
uniform Pallet pal;
uniform Materail materal;
uniform sampler2D zatoon;

vec3 toon(vec3 normal, vec3 frag_position)
{
    normal = normalize(normal);

    vec3 view_dir  = normalize(camera_position - frag_position);
    vec3 light_dir = normalize(light.postion - frag_position);

    float NdotL = clamp((dot(normal, light_dir) + 1.0) * 0.5, 0.0, 1.0);
    float ramp = texture(zatoon, vec2(NdotL, 0.5)).r;

    vec3 light_color = mix(pal.color2, pal.color1, ramp);
    return light_color * light.color;
}

void main()
{
    vec3 color = toon(vs_normal, vs_position);
    FragColor = vec4(color, 1.0);
}