#version 420 core

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
in vec2 vs_texcoord;
in vec4 vs_light_proj_pos;

uniform Light light;
uniform vec3 camera_position;
uniform Pallet pal;

uniform sampler2D zatoon;     
uniform sampler2D shadowMap;  

float shadowCalculation(vec4 fragPosLightSpace)
{
    // Perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projCoords = projCoords * 0.5 + 0.5;

    // If outside the shadow map, treat as not in shadow
    if (projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0 ||
        projCoords.z < 0.0 || projCoords.z > 1.0)
    {
        return 0.0;
    }

    float closestDepth = texture(shadowMap, projCoords.xy).r;
    float currentDepth = projCoords.z;

    // Simple bias to reduce shadow acne
    float bias = 0.0015;

    // 1.0 = in shadow, 0.0 = lit
    return (currentDepth - bias > closestDepth) ? 1.0 : 0.0;
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