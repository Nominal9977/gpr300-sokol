#pragma once

// batteries
#include "batteries/lights.h"
#include "batteries/scene.h"

// ew
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/texture.h"

// std
#include <vector>

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
    std::unique_ptr<ew::Shader> geometry;
    std::unique_ptr<ew::Shader> water;
    std::unique_ptr<ew::Shader> reflection_shader;
    std::unique_ptr<ew::Shader> blinnphong;
    std::unique_ptr<ew::Shader> noprocess;
    std::unique_ptr<ew::Shader> lightsphere;
    std::unique_ptr<ew::Texture> texture;

    batteries::ambient_t ambient;
    batteries::light_t light;
    ew::Mesh sphere;
    ew::Mesh plane;

    float water_y   = 0.521f;
    float suzanne_y = 2.0f;

    std::vector<batteries::light_t> light_instances;
};
