#pragma once

// batteries
#include "batteries/lights.h"
#include "batteries/opengl.h"
#include "batteries/scene.h"

// ew
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/texture.h"

// std
#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
struct TexturedModel
{
  //Directily from LearnOpenGl
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
    };

    //Striped doen vertion of LearnOpengl Mesh
    struct Submesh
    {
        unsigned int vao = 0;
        unsigned int vbo = 0;
        unsigned int ebo = 0;
        int index_count = 0;
        std::string material_name;
    };

    std::vector<Submesh> submeshes;
    std::unordered_map<std::string, std::unique_ptr<ew::Texture>> textures;

    void load(const std::string& path);
    void addTexture(const std::string& mat_name, const std::string& tex_path);
    void draw(ew::Shader* shader) const;
};

class Scene final : public batteries::Scene
{
  public:
    Scene();
    virtual ~Scene();

    void Update(float dt);
    void Render(void);
    void Debug(void);

  private:
    void InitializeInstanceData(void);

  private:
    std::unique_ptr<ew::Model> suzanne;
    std::unique_ptr<ew::Model> Land;
    std::unique_ptr<ew::Model> tree;
    std::unique_ptr<ew::Shader> geometry;
    std::unique_ptr<ew::Shader> water;
    std::unique_ptr<ew::Shader> reflection_shader;
    std::unique_ptr<ew::Shader> blinnphong;
    std::unique_ptr<ew::Shader> fullScreen;
    std::unique_ptr<ew::Texture> wave_tex;
    std::unique_ptr<ew::Texture> wave_warp;
    std::unique_ptr<ew::Texture> tree_tex;
    std::unique_ptr<ew::Texture> land_tex;
    TexturedModel house_model;

    batteries::ambient_t ambient;
    ew::Mesh plane;

    float water_y = 0.521f;
    glm::vec3 suzanne_pos = {0.0f, 2.0f, 0.0f};
    glm::vec3 house_pos = {4.0f, 4.6f, -14.7f};
    float     house_scale = 0.391f;
    std::array<glm::vec3, 5> tree_positions = {{
        { 7.3f,  2.6f,   5.0f},
        {-23.6f, 4.8f,   5.0f},
        {-2.3f,  4.3f, -14.4f},
        {12.0f,  7.0f, -19.4f},
        {16.6f,  7.0f,   3.5f},
    }};
    float tree_scale = 0.036f;

    glm::vec3 fog_color = {0.0f, 0.15f, 0.25f};
    float fog_max_depth = 5.0f;

    float fresnel = 0.05f;
    float refraction_strength = 1.0f;

    // Terrain material
    struct {
        float ambient = 0.4f;
        float diffuse = 1.0f;
        float specular = 0.05f;
        float shininess = 0.05f;
    } terrain_material;

    // Sun light
    float sun_rotiaon   = 30.0f;
    float sun_elevation = 55.0f;
    glm::vec3 sun_color     = {1.0f, 0.9f, 0.8f};
    float sun_intensity = 0.7f;

    std::vector<batteries::light_t> light_instances;

    std::unique_ptr<ew::Shader> depth_shader;
};
