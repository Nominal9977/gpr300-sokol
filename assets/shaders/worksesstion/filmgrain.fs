#version 410

out vec4 FragColor;
in vec2 vUV;

uniform sampler2D uScene;
uniform float uTime;
uniform vec2 uResolution;

uniform float uGrainAmount;


uniform float uVignette;

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = vUV;
    vec3 color = texture(uScene, uv).rgb;

    float n = rand((uv * uResolution) + vec2(uTime * 60.0, uTime * 13.0));
    float grain = (n - 0.5) * 2.0; // [-1,1]
    color += grain * uGrainAmount;

    float dist = distance(uv, vec2(0.5));
    float vig = smoothstep(0.85, 0.35, dist);
    color *= mix(1.0, vig, clamp(uVignette, 0.0, 1.0));

    FragColor = vec4(color, 1.0);
}