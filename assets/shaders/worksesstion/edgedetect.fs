#version 410

out vec4 FragColor;

in vec2 vUV;

uniform sampler2D uScene;
uniform vec2 uResolution;

void main()
{
    vec2 texel = 1.0 / uResolution;

    float kernelX[9] = float[](
        -1, 0, 1,
        -2, 0, 2,
        -1, 0, 1
    );

    float kernelY[9] = float[](
        -1,-2,-1,
         0, 0, 0,
         1, 2, 1
    );

    vec2 offsets[9] = vec2[](
        vec2(-texel.x,  texel.y),
        vec2(0.0,       texel.y),
        vec2(texel.x,   texel.y),

        vec2(-texel.x,  0.0),
        vec2(0.0,       0.0),
        vec2(texel.x,   0.0),

        vec2(-texel.x, -texel.y),
        vec2(0.0,      -texel.y),
        vec2(texel.x,  -texel.y)
    );

    float gx = 0.0;
    float gy = 0.0;

    for(int i = 0; i < 9; i++)
    {
        vec3 sampleColor =
            texture(uScene, vUV + offsets[i]).rgb;

        float intensity =
            dot(sampleColor, vec3(0.299,0.587,0.114));

        gx += intensity * kernelX[i];
        gy += intensity * kernelY[i];
    }

    float edge = length(vec2(gx, gy));

    FragColor = vec4(vec3(edge), 1.0);
}