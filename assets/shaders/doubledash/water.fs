#version 300 es

precision mediump float;

out vec4 FragColor;

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;


uniform sampler2D wave_tex;
uniform sampler2D wave_spec;
uniform sampler2D wave_warp;


uniform vec3 camera_position;
uniform float time;
uniform vec3 water_color;

float scale = 1.0f;


void main()
{

  //warp:
  vec2 warp_uv =  vs_texcoord *scale;
  vec2 warp_scroll = vec2(0.5, 0.5) * time;
  vec2 warp = texture(wave_warp, warp_uv + warp_scroll).xy;

  vec2 albedo_uv =  vs_texcoord * scale;
  vec4 albedo = texture(wave_tex, albedo_uv + warp).rgba;

  vec3 final_color = water_color + vec3(albedo.a);
  //shimmer
  vec2 spec_uv =  vs_texcoord *scale;
  vec2 spec_scroll = vec2(0.5, 0.5) * time;

  vec3 spec_sample1 = texture(wave_spec, spec_uv + vec2(0.5, 0.5) * time).rgb;
  vec3 spec_sample2 = texture(wave_spec, spec_uv + vec2(-0.5, -0.5) * time).rgb;
  vec3 spec = spec_sample1 + spec_sample2;


  float fresnel = dot(normalize(camera_position), vec3( 0.0, 1.0, 0.0));
  const vec3 bright = vec3(0.299, 0.587, 0.114);
  float brightness = dot(spec, bright);

float lower = 0.85;
float upeper  = 95.0;
  if(brightness <= lower|| brightness >= upeper)
  {
    final_color = mix(water_color, final_color + spec , fresnel );
  }



  FragColor = vec4(final_color, 1.0);
}