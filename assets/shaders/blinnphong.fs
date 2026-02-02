#version 410

precision mediump float;

out vec4 FragColor;

struct Light{
  vec3 color;
  vec3 postion;
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

uniform texture0 texture0
uniform Light light;
uniform vec3 camera_position;
uniform Materail materal;


vec3 blinnphong(vec3 normal, vec3 frag_postion, Light light, Materail materal) {
    vec3 view_dir = normalize(camera_position - frag_postion);
    vec3 light_dir = normalize(light.postion - frag_postion);
    vec3 reflect_dir = reflect(light_dir, vs_normal);
    vec3 half_dir = normalize(light_dir + view_dir);

    //materal lighting 

    float NdotL =  max(dot(normal, light_dir), 0.0);
    float NdotH = pow(max(dot(normal, half_dir), 0.0), materal.shinniness);

    vec3 diffuse = NdotL * materal.diffuse;
    vec3 specular =  light.color * NdotH * materal.specular;

    vec3 lighting =  diffuse + vec3(specular);
    return  lighting * light.color;
}

void main()
{
  vec3 ambient = vec3(1.0);
  vec3 color = blinnphong(vs_normal, vs_position, light, materal)+ambient * 0.5;

   vec3 object_color = texture(texture0, vs_texcoord).rgb;

  vec3 object_color = vs_normal.rgb *0.5 +0.5;
  vec3 final_color = object_color*color;
  FragColor = vec4(final_color, 1.0);
}