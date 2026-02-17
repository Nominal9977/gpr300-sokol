#version 300 es

precision mediump float;

out vec4 FragColor;

in vec2 vs_texcoord;

uniform sampler2D screen;

const float offset = 1.0/ 300.0;


const vec2 offsets[9] = vec2[]
(
    vec2(-offset,offset), //top-left
    vec2(0, offset), //top-middle
    vec2(offset,offset), //top-right

    vec2(-offset,0), //center-left
    vec2(0,0), //center-middle
    vec2(offset,0), //center-right

    vec2(-offset, -offset), //bottom-left
    vec2(0, -offset), //bottom-middle
    vec2(offset, -offset) //bottom-right
);
uniform float strength;
const float kernel[9] = float[](
    1.0,2.0,1.0,
    2.0,4.0,2.0,
    1.0,2.0,1.0
);

void main()
{
  //vec3 color = texture(screen, vs_texcoord).rgb;
  vec3 color = vec3(0,0,0);
  for(int i = 0; i < 9; i++)
  {
    vec3 local = vec3(texture(screen, vs_texcoord + offsets[i]));
    color += local * (kernel[i]/strength);
  }

  vec3 grey = vec3(dot(color, vec3(0.299, 0.587, 0.114)));
  FragColor = vec4(grey, 1.0);
}