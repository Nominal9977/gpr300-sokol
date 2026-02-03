#version 410

precision mediump float;

out vec4 FragColor;

struct Light{
  vec3 color;
  vec3 postion;
};

struct Pallet{
vec3 color1;
vec3 color2;
};

struct Materail{
  vec3 ambeint;
  vec3 diffuse;
  vec3 specular;
  float shinniness;
};

// varyings
in vec3 vs_position;
in vec3 vs_normal;
in vec2 vs_texcoord;

uniform Light light;
uniform vec3 camera_position;
uniform Pallet pal;
uniform Materail materal;
uniform sampler2D zatoon;


vec3 toon(vec3 normal, vec3 frag_postion, Light light, Materail materal) {
    vec3 view_dir = normalize(camera_position - frag_postion);
    vec3 light_dir = normalize(light.postion - frag_postion);
    vec3 reflect_dir = reflect(light_dir, vs_normal);
    vec3 half_dir = normalize(light_dir + view_dir);

    //materal lighting 

    float NdotL =  (dot(normal, light_dir)+ 1.0) *0.5;
    float NdotH = max(dot(normal, half_dir), 0.0);

    


    vec3 gradint = texture(zatoon, vec2(NdotL, NdotL)).rgb;

    vec3 light_color = mix(pal.color2, pal.color1, gradint);
    // vec3 diffuse = NdotL * materal.diffuse;
    // vec3 specular =  light.color * NdotH * materal.specular;

    // vec3 lighting =  diffuse + vec3(specular);
    return  light_color;
}

void main()
{
  //vec3 ambient = vec3(1.0);
  vec3 color = toon(vs_normal, vs_position, light, materal) * 0.5;
  //vec3 object_color = texture(texture0, vs_texcoord).rgb;
  vec3 object_color = vs_normal.rgb *0.5 +0.5;
  vec3 final_color = color;
  FragColor = vec4(final_color, 1.0);
}