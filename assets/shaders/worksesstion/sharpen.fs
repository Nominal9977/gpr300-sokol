#version 410

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uScene;
uniform vec2 uResolution;

uniform float uStrength;

void main()
{
    vec2 uv = vUV;
    vec2 texel = 1.0 / uResolution;

    vec3 c  = texture(uScene, uv).rgb;
    vec3 up = texture(uScene, uv + vec2(0.0,  texel.y)).rgb;
    vec3 dn = texture(uScene, uv + vec2(0.0, -texel.y)).rgb;
    vec3 lf = texture(uScene, uv + vec2(-texel.x, 0.0)).rgb;
    vec3 rt = texture(uScene, uv + vec2( texel.x, 0.0)).rgb;

    float s = max(uStrength, 0.0);
    vec3 sharpened = c * (1.0 + 4.0 * s) - (up + dn + lf + rt) * s;

    FragColor = vec4(sharpened, 1.0);
}