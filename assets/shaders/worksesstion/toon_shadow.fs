#version 410

out vec4 FragColor;

struct Light {
    vec3 color;
    vec3 postion; 
};

struct Pallet {
    vec3 color1;
    vec3 color2;
};

in vec3 vs_position;
in vec3 vs_normal;
// in vec2 vs_texcoord;
in vec4 vs_light_proj_pos;

uniform Light light;
uniform vec3 camera_position;
uniform Pallet pal;
uniform float bais;

uniform sampler2D zatoon;     
uniform sampler2D shadowMap;  

float shadowCalculation(vec4 fragPosLightSpace)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    float Colosest_depth = texture(shadowMap, projCoords.xy).r;
    float current_depth = projCoords.z;

    float shodow = (current_depth - bais > Colosest_depth) ? 1.0 : 0.0;

    return shodow;

}

vec3 toon(vec3 normal, vec3 frag_position)
{
    normal = normalize(normal);

    vec3 light_dir = normalize(light.postion - frag_position);

    float NdotL = clamp((dot(normal, light_dir) + 1.0) * 0.5, 0.0, 1.0);
    float ramp = texture(zatoon, vec2(NdotL, 0.5)).r;

    vec3 toonLight = mix(pal.color2, pal.color1, ramp);
    return toonLight * light.color;
}

void main()
{
    vec3 col = toon(vs_normal, vs_position);

    float shadow = shadowCalculation(vs_light_proj_pos);
    col *= (1.0 - shadow);

    FragColor = vec4(col, 1.0);
}