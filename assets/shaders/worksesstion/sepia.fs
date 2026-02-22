#version 410
out vec4 FragColor;
in vec2 vUV;
uniform sampler2D uScene;

void main()
{
    vec3 c = texture(uScene, vUV).rgb;

    vec3 sep;
    sep.r = dot(c, vec3(0.393, 0.769, 0.189));
    sep.g = dot(c, vec3(0.349, 0.686, 0.168));
    sep.b = dot(c, vec3(0.272, 0.534, 0.131));

    FragColor = vec4(clamp(sep, 0.0, 1.0), 1.0);
}