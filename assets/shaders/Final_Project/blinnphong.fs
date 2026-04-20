#version 410
// blinn-phong fragment shader - calculates point light diffuse and specular from g-buffer data

precision mediump float;

out vec4 FragColor;

struct Light {
    vec3 position;
    vec3 color;
    float radius;
};

// g-buffer textures
uniform sampler2D g_position;
uniform sampler2D g_normal;
uniform sampler2D g_albedo;
uniform sampler2D g_material;

uniform vec3 camera_position;
uniform Light light;

in vec2 vUV;

vec3 blinnphong(vec3 frag_position, vec3 normal, vec3 albedo, vec4 mat_data, Light l)
{
    float diffuse_k  = mat_data.g;  // diffuse strength
    float specular_k = mat_data.b;  // specular strength 
    float shininess  = mat_data.a * 128.0 + 1.0; // make shininess into usable range

    float dist  = length(l.position - frag_position); // distance from light
    float atten = clamp(1.0 - (dist / l.radius), 0.0, 1.0);  // falloff
    atten      *= atten;  // squared for smoother falloff

    vec3 view_dir  = normalize(camera_position - frag_position); // get direction to camera
    vec3 light_dir = normalize(l.position - frag_position); // get direction to light
    vec3 half_dir  = normalize(light_dir + view_dir); // vector for specular

    float NdotL = max(dot(normal, light_dir), 0.0);// diffuse angle
    float NdotH = pow(max(dot(normal, half_dir), 0.0), shininess); // specular highlight

    vec3 diffuse  = NdotL * diffuse_k * albedo; // diffuse color contribution
    vec3 specular = NdotH * specular_k * vec3(1.0);  //specular highlight

    return l.color * (diffuse + specular) * atten;
}

void main()
{
    vec3 frag_position = texture(g_position, vUV).xyz;  // read world position 
    vec3 normal        = normalize(texture(g_normal, vUV).xyz); // read and normalize normal
    vec3 albedo        = texture(g_albedo, vUV).rgb;   // read surface color
    vec4 mat_data      = texture(g_material, vUV);     // read material properties

    FragColor = vec4(blinnphong(frag_position, normal, albedo, mat_data, light), 1.0);
}
