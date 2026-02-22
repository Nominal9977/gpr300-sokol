#version 410

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uScene;
uniform vec2 uResolution;
uniform float uPixelSize;

void main()
{
    vec2 uv = vUV;
    vec2 pixel = uv * uResolution;
    pixel = floor(pixel / max(uPixelSize, 1.0)) * max(uPixelSize, 1.0);
    vec2 snappedUV = pixel / uResolution;

    vec3 color = texture(uScene, snappedUV).rgb;
    FragColor = vec4(color, 1.0);
}