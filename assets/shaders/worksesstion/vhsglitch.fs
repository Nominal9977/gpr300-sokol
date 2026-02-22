#version 410

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uScene;
uniform float uTime;
uniform vec2 uResolution;

uniform float uStrength;

float rand(vec2 co)
{
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

void main()
{
    vec2 uv = vUV;


    float line = floor(uv.y * uResolution.y);
    float lineNoise = rand(vec2(line, floor(uTime * 30.0)));
    float jitter = (lineNoise - 0.5) * 2.0;                
    float jitterAmt = 2.0 * uStrength / uResolution.x;      
    uv.x += jitter * jitterAmt;

    float jump = step(0.96, rand(vec2(floor(uTime * 2.0), 123.0))); 
    uv.y += jump * (rand(vec2(uTime, 77.7)) - 0.5) * 0.08 * uStrength;

    uv = clamp(uv, vec2(0.0), vec2(1.0));

    vec2 rgbOff = vec2(1.0, 0.0) * (1.5 * uStrength / uResolution.x);

    float r = texture(uScene, uv + rgbOff).r;
    float g = texture(uScene, uv).g;
    float b = texture(uScene, uv - rgbOff).b;

    vec3 color = vec3(r, g, b);

    float scan = sin(uv.y * uResolution.y * 3.14159);
    color -= scan * (0.04 * uStrength);

    float n = rand(uv * uResolution.xy + uTime * 60.0);
    color += (n - 0.5) * (0.10 * uStrength);

    color *= 0.95;

    FragColor = vec4(color, 1.0);
}