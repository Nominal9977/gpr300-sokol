#version 410

precision mediump float;

out vec4 frag_color;

uniform vec3 color;

void main()
{
    frag_color = vec4(color.rgb, 1.0);
}