#version 410

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uScene;

uniform float uLevels;

void main()
{
    vec2 uv = vUV;

    vec3 color = texture(uScene, uv).rgb;

    float levels = max(uLevels, 2.0);
    
    color = floor(color * levels) / levels;

    FragColor = vec4(color, 1.0);
}