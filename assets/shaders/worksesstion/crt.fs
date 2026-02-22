#version 410

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uScene;
uniform float uTime;
uniform float uCurve;
uniform float uScanlineStrength;
uniform float uRGBSplit;
uniform float uVignette;

vec2 barrelDistort(vec2 uv, float k)
{

    vec2 p = uv * 2.0 - 1.0;
    float r2 = dot(p, p);
    p *= (1.0 + k * r2);

    return (p * 0.5 + 0.5);
}

void main()
{
    vec2 uv = vUV;

    uv = barrelDistort(uv, uCurve);

    if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0)
    {
        FragColor = vec4(0,0,0,1);
        return;
    }


    vec2 split = vec2(uRGBSplit) / vec2(800.0, 600.0);

    float r = texture(uScene, uv + split).r;
    float g = texture(uScene, uv).g;
    float b = texture(uScene, uv - split).b;
    vec3 color = vec3(r, g, b);

    float scan = sin((uv.y * 600.0) * 3.14159);
    color -= scan * uScanlineStrength;

    float flicker = 0.98 + sin(uTime * 20.0) * 0.02;
    color *= flicker;

    float dist = distance(uv, vec2(0.5));
    float vig = smoothstep(0.8, 0.35, dist);
    color *= mix(1.0, vig, uVignette);

    FragColor = vec4(color, 1.0);
}