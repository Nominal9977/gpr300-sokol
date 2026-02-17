#include "scene.h"
// imgui
#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"

// glm
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

// batteries
#include "batteries/opengl.h"
#include <iostream>

struct FullScreenQuad
{
    GLuint vao;
    GLuint vbo;

    void Initialize()
    {
        float vertices[] = {
            // pos (x, y),
            // texcoord (u, v)
            // triangle 1
            -1.0f, 1.0f, 0.0f, 1.0f,   
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f, -1.0f, 1.0f, 0.0f,

            // triangle 2
            -1.0f, 1.0f, 0.0f, 1.0f,
            1.0f, -1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
        };

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

        // pos (x, y),
        // texcoord (u, v)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

        // always last.
        glBindVertexArray(0);
    }
};
FullScreenQuad fullscreen;

Scene::Scene()
{

    glEnable(GL_DEPTH_TEST);
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    
    toon = std::make_unique<ew::Shader>(
        "assets/shaders/defualt.vs",
        "assets/shaders/toon.fs"
    );

    default_pp =  std::make_unique<ew::Shader>(
        "assets/shaders/fullscreen.vs",
        "assets/shaders/blur.fs");
    texture = std::make_unique<ew::Texture>("assets/textures/ZAtoon.png");


    light = {
        .brightness = 0.1f,
        .color = { 1.0f, 1.0f, 1.0f },
        .position = { 2.0f, 0.0f, 1.0f },
    };

    fullscreen.Initialize();
    // frameBuffer setup

    glCreateFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {
        //create texture
        glGenTextures(1, &fbo_texture);
        glBindTexture(GL_TEXTURE_2D, fbo_texture);

        //Create 800/600 render texture with 8 unsigned bytes
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);

        //Create depth texture
        glGenTextures(1, &fbo_depth);
        glBindTexture(GL_TEXTURE_2D, fbo_depth);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fbo_depth, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
        std::cout<<"Error no frame buffer";
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

 

}



struct{
    float shinniness = 128.9;
    glm::vec3 ambent =  glm::vec3(0.0f);
    glm::vec3 diffuse =  glm::vec3(0.0f);
    glm::vec3 specular =  glm::vec3(0.0f);
    float strength = 1.0f;
} debug;



Scene::~Scene()
{
    glDeleteFramebuffers(1, &fbo);
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);

    /* body */
}


void Scene::Render(void)
{
//     glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
//     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {

        
        const auto view_proj = camera.Projection() * camera.View();


            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            glEnable(GL_DEPTH_TEST);
            // glDisable(GL_DEPTH_TEST);

            auto index = 0;
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D, texture->getID());

            toon->use();

            // scene matrices
            toon->setMat4("model", glm::mat4(1.0f));
            toon->setMat4("view_proj", view_proj);

            toon->setInt("zatoon", index);


            toon->setVec3("light.color", light.color);
            toon->setVec3("camera_position", camera.position);
            toon->setVec3("light.postion", light.position);
            toon->setVec3("materal.ambeint", debug.ambent);
            toon->setVec3("materal.diffuse", debug.diffuse);
            toon->setVec3("materal.specular", debug.specular);

            toon->setVec3("pal.color1", {1.0f, 0.6f, 0.2f});
            toon->setVec3("pal.color2", {1.0f, 0.23f, 0.9f});
            toon->setFloat("materal.shinniness", debug.shinniness);


            // draw suzanne
            suzanne->draw();
            
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    default_pp -> use();
    default_pp -> setInt("screen", 0);
    default_pp -> setFloat("strength", debug.strength);

    glDisable(GL_DEPTH_TEST);
    
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindVertexArray(fullscreen.vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fbo_texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
    ImGui::SliderFloat("Blur Stenght", &debug.strength, 0.0f, 300.0f);
    ImGui::ColorEdit3("Light Color", &light.color[0]);
    ImGui::DragFloat("shinniness", &debug.shinniness, 1.0f, 0.0f, 128.9f);
    ImGui::ColorEdit3("Materal Ambeint", &debug.ambent[0]);
    ImGui::ColorEdit3("Materal Diffuse", &debug.diffuse[0]);
    ImGui::ColorEdit3("Materal Specular", &debug.specular[0]);


    /* build debug ui here */

    ImGui::Image(
        (void*)(intptr_t)fbo_texture,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

    ImGui::Image(
        (void*)(intptr_t)fbo_depth,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0));

    ImGui::End();
}