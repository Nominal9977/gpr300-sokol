#version 410

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uScene;
uniform float uTime;

float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = vUV;
    vec3 color = texture(uScene, uv).rgb;

    float gray = dot(color, vec3(0.299, 0.587, 0.114));
    color = vec3(gray);

    float scanline = sin(uv.y * 800.0) * 0.04;
    color -= scanline;

    float noise = rand(uv + uTime * 0.5);
    color += noise * 0.08;

    float flicker = 0.97 + sin(uTime * 20.0) * 0.03;
    color *= flicker;

    float dist = distance(uv, vec2(0.5));
    float vignette = smoothstep(0.8, 0.4, dist);
    color *= vignette;

    FragColor = vec4(color, 1.0);
}