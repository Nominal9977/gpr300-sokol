#version 300 es

precision mediump float;

out vec4 FragColor;

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;


uniform sampler2D tex;
uniform float time;
uniform vec3 water_color;


void main()
{
  vec2 dir = vec2(1.0, 0.0);
  vec2 uv = vs_texcoord +vec2(time*dir);
  uv.x += 0.01 * sin(uv.x *3.5 + time);
  uv.y += -0.35 * sin(uv.x *1.5 + time);

  vec4 sample1 = texture(tex, uv *1.0);
  vec4 sample2 = texture(tex, uv *1.2);

  vec3 object_color = vec3(
    (sample1 * 0.75) - (sample2 *0.25));
  FragColor = vec4(object_color + water_color, 1.0);
}