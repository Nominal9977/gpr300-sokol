#version 410

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uScene;
uniform vec2 uResolution;
uniform float uStrength;

void main()
{
    vec2 uv = vUV;

    vec2 center = vec2(0.5);
    vec2 dir = uv - center;

    vec2 offset = dir * (uStrength / uResolution);

    float r = texture(uScene, uv + offset).r;
    float g = texture(uScene, uv).g;
    float b = texture(uScene, uv - offset).b;

    vec3 color = vec3(r, g, b);

    FragColor = vec4(color, 1.0);
}