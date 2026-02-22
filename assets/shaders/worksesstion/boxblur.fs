#version 410

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uScene;
uniform float uBlurStrength;
uniform vec2 uResolution;

void main()
{
    vec2 uv = vUV;

    vec2 texel = 1.0 / uResolution;
    vec2 stepUV = texel * max(uBlurStrength, 0.0);
    vec3 sum = vec3(0.0);

    sum += texture(uScene, uv + vec2(-stepUV.x,  stepUV.y)).rgb;
    sum += texture(uScene, uv + vec2( 0.0,       stepUV.y)).rgb;
    sum += texture(uScene, uv + vec2( stepUV.x,  stepUV.y)).rgb;

    sum += texture(uScene, uv + vec2(-stepUV.x,  0.0)).rgb;
    sum += texture(uScene, uv + vec2( 0.0,       0.0)).rgb;
    sum += texture(uScene, uv + vec2( stepUV.x,  0.0)).rgb;

    sum += texture(uScene, uv + vec2(-stepUV.x, -stepUV.y)).rgb;
    sum += texture(uScene, uv + vec2( 0.0,      -stepUV.y)).rgb;
    sum += texture(uScene, uv + vec2( stepUV.x, -stepUV.y)).rgb;

    vec3 color = sum / 9.0;
    FragColor = vec4(color, 1.0);
}