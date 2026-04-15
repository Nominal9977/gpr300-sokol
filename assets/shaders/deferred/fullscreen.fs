#version 410

out vec4 FragColor;
in vec2 vUV;

uniform sampler2D screen;
uniform sampler2D g_albedo;
uniform vec3 ambient_color;
uniform float ambient_strength;

void main()
{
    vec3 color  = texture(screen, vUV).rgb;
    vec3 albedo = texture(g_albedo, vUV).rgb;
    vec3 ambient = ambient_color * albedo * ambient_strength;
    FragColor = vec4(color + ambient, 1.0);
}