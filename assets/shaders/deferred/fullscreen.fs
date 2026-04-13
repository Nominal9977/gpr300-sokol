#version 410

out vec4 FragColor;
in vec2 vUV;

uniform sampler2D screen;

void main()
{
    vec3 color = texture(screen, vUV).rgb;
    FragColor = vec4(color, 1.0);
}