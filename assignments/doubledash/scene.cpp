#include "scene.h"

// imgui
#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// batteries
#include "batteries/opengl.h"

#include "ew/procGen.h"

Scene::Scene()
{
     water = std::make_unique<ew::Shader>(
        "assets/shaders/doubledash/water.vs",
        "assets/shaders/doubledash/water.fs"
    );

    light = {
        .brightness = 0.1f,
        .color = { 1.0f, 1.0f, 1.0f },
        .position = { 2.0f, 0.0f, 1.0f },
    };

    
    wave_spec = std::make_unique<ew::Texture>("assets/doubledash/wave_spec.png");
    wave_tex = std::make_unique<ew::Texture>("assets/doubledash/wave_tex.png");
    wave_warp = std::make_unique<ew::Texture>("assets/doubledash/wave_warp.png");

    plane.load(ew::createPlane(100.0f, 100.0f, 10.0f));

}

struct{
    float shinniness = 128.9;
    glm::vec3 ambent =  {0.3f , 0.56f, 0.74f};
    glm::vec3 diffuse =  {0.6f , 0.2f, 0.74f};
    glm::vec3 specular =  glm::vec3(0.0f);
} debug;

struct {
    glm::vec3 water_color = {0.3, 0.2, 0.5};
}watercol;



Scene::~Scene()
{
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}


void Scene::Render(void)
{
    const auto view_proj = camera.Projection() * camera.View();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    // glDisable(GL_DEPTH_TEST);


     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_2D, wave_tex->getID());

     glActiveTexture(GL_TEXTURE1);
     glBindTexture(GL_TEXTURE_2D, wave_warp->getID());

     glActiveTexture(GL_TEXTURE2);
     glBindTexture(GL_TEXTURE_2D, wave_spec->getID());

    water->use();

    // scene matrices
    water->setMat4("model", glm::mat4(1.0f));
    water->setMat4("view_proj", view_proj);

    water->setInt("wave_tex", 0);
    water->setInt("wave_spec", 1);
    water->setInt("wave_warp", 2);

    
    water->setFloat("time", (float)time.absolute);

    water->setVec3("water_color", watercol.water_color);
    water->setVec3("light.color", light.color);
    water->setVec3("camera_position", camera.position);
    water->setVec3("light.postion", light.position);
    water->setVec3("materal.ambeint", debug.ambent);
    water->setVec3("materal.diffuse", debug.diffuse);
    water->setVec3("materal.specular", debug.specular);
    water->setFloat("materal.shinniness", debug.shinniness);
}

void Scene::Debug(void)
{
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    glm::mat4 m{1.0f};
    auto *view = glm::value_ptr(camera.View());
    auto *proj = glm::value_ptr(camera.Projection());
    
    ImGuizmo::DrawGrid(view, proj, glm::value_ptr(m), 100.0f);

    auto matrix = glm::translate(glm::mat4(1.0f), light.position);

    ImGuizmo::Manipulate(
        view,
        proj,
        ImGuizmo::TRANSLATE,
        ImGuizmo::WORLD,
        glm::value_ptr(matrix)
    );

    if(ImGuizmo::IsUsing())
    {
        light.position = glm::vec3(matrix[3]);
    }

    cameracontroller.Debug();

    ImGui::Begin("Controlls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Paused", &time.paused);
    ImGui::SliderFloat("Time Factor", &time.factor, 0.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", &light.color[0]);
    ImGui::DragFloat("shinniness", &debug.shinniness, 1.0f, 0.0f, 128.9f);
    ImGui::ColorEdit3("Materal Ambeint", &debug.ambent[0]);
    ImGui::ColorEdit3("Materal Diffuse", &debug.diffuse[0]);
    ImGui::ColorEdit3("Water Color", &watercol.water_color[0]);

    plane.draw();


    /* build debug ui here */

    ImGui::End();
}