#include "scene.h"

#include "imgui/imgui.h"
#include "imguizmo/imguizmo.h"


#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "batteries/opengl.h"

#include <iostream>
#include <vector>
#include <algorithm>
struct FullScreenQuad
{
    GLuint vao = 0;
    GLuint vbo = 0;

    void Initialize()
    {
        // pos (x, y), uv (u, v)
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

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void Destroy()
    {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
        vao = 0;
        vbo = 0;
    }
};

static FullScreenQuad fullscreen;
static struct
{
    float shininess = 32.0f;
    glm::vec3 ambient  = glm::vec3(0.2f);
    glm::vec3 diffuse  = glm::vec3(1.0f);
    glm::vec3 specular = glm::vec3(0.2f);

    float boxBlurStrength = 1.0f;       
    float sharpenStrength = 1.0f;        
    float pixelSize = 12.0f;       
    float chromaStrength = 3.5f;        

    float crtCurve = 0.08f;            
    float crtScanlineStrength = 0.06f;   
    float crtRGBSplit = 1.2f;          
    float crtVignette = 0.8f;            

    float vhsStrength = 0.6f;           

    float posterizeLevels = 6.0f;        
    float grainAmount = 0.08f;           
    float grainVignette = 0.8f;          
} debug;

enum PostFX : int
{
    PP_None = 0,
    PP_CCTV,
    PP_BoxBlur,
    PP_EdgeDetect,
    PP_Sharpen,
    PP_Pixelate,
    PP_ChromaticAberration,
    PP_CRT,
    PP_Sepia,
    PP_VHSGlitch,
    PP_Posterize,
    PP_FilmGrain,
    PP_Count
};

Scene::Scene()
{
    glEnable(GL_DEPTH_TEST);

    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");

    toonShader = std::make_unique<ew::Shader>(
        "assets/shaders/default.vs",
        "assets/shaders/worksesstion/toon.fs"
    );

    texture = std::make_unique<ew::Texture>("assets/textures/ZAtoon.png");

    light = {
        .brightness = 0.1f,
        .color = { 1.0f, 1.0f, 1.0f },
        .position = { 2.0f, 0.0f, 1.0f },
    };

    fullscreen.Initialize();

    mPostShaders.clear();
    mPostShaders.reserve(16);

    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/fullscreen.fs"));                 
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/cctv.fs"));          
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/boxblur.fs"));       
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/edgedetect.fs"));   
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/sharpen.fs"));       
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/pixelate.fs"));     
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/chromaticaberration.fs")); 
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/crt.fs"));           
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/sepia.fs"));         
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/vhsglitch.fs"));     
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/posterize.fs"));     
    mPostShaders.push_back(std::make_unique<ew::Shader>("assets/shaders/fullscreen.vs", "assets/shaders/worksesstion/filmgrain.fs"));    

    mSelectedPost = PP_None;

    glCreateFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {

        glGenTextures(1, &fbo_texture);
        glBindTexture(GL_TEXTURE_2D, fbo_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture, 0);

        glGenTextures(1, &fbo_depth);
        glBindTexture(GL_TEXTURE_2D, fbo_depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 800, 600, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, fbo_depth, 0);

        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "Error: framebuffer incomplete\n";
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Scene::~Scene()
{
    fullscreen.Destroy();

    if (fbo_depth)   glDeleteTextures(1, &fbo_depth);
    if (fbo_texture) glDeleteTextures(1, &fbo_texture);
    if (fbo)         glDeleteFramebuffers(1, &fbo);

    fbo_depth = 0;
    fbo_texture = 0;
    fbo = 0;
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);
}

void Scene::Render(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    {
        const auto view_proj = camera.Projection() * camera.View();

        glViewport(0, 0, 800, 600);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glEnable(GL_DEPTH_TEST);

        const int texUnit = 0;
        glActiveTexture(GL_TEXTURE0 + texUnit);
        glBindTexture(GL_TEXTURE_2D, texture->getID());

        toonShader->use();

        toonShader->setMat4("model", glm::mat4(1.0f));
        toonShader->setMat4("view_proj", view_proj);

        toonShader->setInt("zatoon", texUnit);
        toonShader->setInt("toonRamp", texUnit);
        toonShader->setInt("rampTex", texUnit);
        toonShader->setInt("uRamp", texUnit);
        toonShader->setInt("uToonRamp", texUnit);

        toonShader->setVec3("light.color", light.color);
        toonShader->setVec3("camera_position", camera.position);
        toonShader->setVec3("light.position", light.position);

        toonShader->setVec3("material.ambient",  debug.ambient);
        toonShader->setVec3("material.diffuse",  debug.diffuse);
        toonShader->setVec3("material.specular", debug.specular);
        toonShader->setFloat("material.shininess", debug.shininess);

        toonShader->setVec3("materal.ambeint",   debug.ambient);
        toonShader->setVec3("materal.diffuse",   debug.diffuse);
        toonShader->setVec3("materal.specular",  debug.specular);
        toonShader->setFloat("materal.shinniness", debug.shininess);
        toonShader->setVec3("light.postion", light.position);

        toonShader->setVec3("pal.color1", { 1.0f, 0.6f, 0.2f });
        toonShader->setVec3("pal.color2", { 1.0f, 0.23f, 0.9f });

        suzanne->draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0, 0, 800, 600);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (!mPostShaders.empty())
    {
        const int ppIdx = std::clamp(mSelectedPost, 0, (int)mPostShaders.size() - 1);
        ew::Shader* pp = mPostShaders[ppIdx].get();

        pp->use();
        pp->setInt("uScene", 0);
        pp->setVec2("uResolution", glm::vec2(800.0f, 600.0f));
        pp->setFloat("uTime", (float)time.absolute);

        switch ((PostFX)ppIdx)
        {
            case PP_None:
                break;

            case PP_CCTV:
                break;

            case PP_BoxBlur:
                pp->setFloat("uBlurStrength", debug.boxBlurStrength);
                break;

            case PP_EdgeDetect:

                break;

            case PP_Sharpen:
                pp->setFloat("uStrength", debug.sharpenStrength);
                break;

            case PP_Pixelate:
                pp->setFloat("uPixelSize", debug.pixelSize);
                break;

            case PP_ChromaticAberration:
                pp->setFloat("uStrength", debug.chromaStrength);
                break;

            case PP_CRT:
                pp->setFloat("uCurve", debug.crtCurve);
                pp->setFloat("uScanlineStrength", debug.crtScanlineStrength);
                pp->setFloat("uRGBSplit", debug.crtRGBSplit);
                pp->setFloat("uVignette", debug.crtVignette);
                break;

            case PP_Sepia:
                break;

            case PP_VHSGlitch:
                pp->setFloat("uStrength", debug.vhsStrength);
                break;

            case PP_Posterize:
                pp->setFloat("uLevels", debug.posterizeLevels);
                break;

            case PP_FilmGrain:
                pp->setFloat("uGrainAmount", debug.grainAmount);
                pp->setFloat("uVignette", debug.grainVignette);
                break;

            default:
                break;
        }

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fbo_texture);

        glBindVertexArray(fullscreen.vao);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
    }
}

void Scene::Debug(void)
{
    ImGuizmo::BeginFrame();
    ImGuizmo::SetDrawlist(ImGui::GetBackgroundDrawList());
    ImGuizmo::SetRect(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

    glm::mat4 m{ 1.0f };
    auto* view = glm::value_ptr(camera.View());
    auto* proj = glm::value_ptr(camera.Projection());

    ImGuizmo::DrawGrid(view, proj, glm::value_ptr(m), 100.0f);

    auto matrix = glm::translate(glm::mat4(1.0f), light.position);

    ImGuizmo::Manipulate(
        view,
        proj,
        ImGuizmo::TRANSLATE,
        ImGuizmo::WORLD,
        glm::value_ptr(matrix)
    );

    if (ImGuizmo::IsUsing())
    {
        light.position = glm::vec3(matrix[3]);
    }

    cameracontroller.Debug();

    ImGui::Begin("Controls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Paused", &time.paused);
    ImGui::SliderFloat("Time Factor", &time.factor, 0.0f, 10.0f);
    ImGui::ColorEdit3("Light Color", &light.color[0]);

    const char* postNames[] = {
        "None",
        "CCTV",
        "Box Blur",
        "Edge Detect",
        "Sharpen",
        "Pixelate",
        "Chromatic Aberration",
        "CRT",
        "Sepia",
        "VHS Glitch",
        "Posterize",
        "Film Grain"
    };
    ImGui::Combo("Post Process", &mSelectedPost, postNames, IM_ARRAYSIZE(postNames));

    ImGui::Separator();
    ImGui::Text("Effect Settings:");

    switch ((PostFX)mSelectedPost)
    {
        case PP_BoxBlur:
            ImGui::SliderFloat("Blur Strength", &debug.boxBlurStrength, 0.0f, 5.0f);
            break;

        case PP_Sharpen:
            ImGui::SliderFloat("Sharpen Strength", &debug.sharpenStrength, 0.0f, 3.0f);
            break;

        case PP_Pixelate:
            ImGui::SliderFloat("Pixel Size", &debug.pixelSize, 1.0f, 64.0f);
            break;

        case PP_ChromaticAberration:
            ImGui::SliderFloat("Chroma Strength", &debug.chromaStrength, 0.0f, 10.0f);
            break;

        case PP_CRT:
            ImGui::SliderFloat("Curve", &debug.crtCurve, 0.0f, 0.2f);
            ImGui::SliderFloat("Scanline Strength", &debug.crtScanlineStrength, 0.0f, 0.2f);
            ImGui::SliderFloat("RGB Split", &debug.crtRGBSplit, 0.0f, 4.0f);
            ImGui::SliderFloat("Vignette", &debug.crtVignette, 0.0f, 1.0f);
            break;

        case PP_VHSGlitch:
            ImGui::SliderFloat("VHS Strength", &debug.vhsStrength, 0.0f, 1.0f);
            break;

        case PP_Posterize:
            ImGui::SliderFloat("Levels", &debug.posterizeLevels, 2.0f, 32.0f);
            break;

        case PP_FilmGrain:
            ImGui::SliderFloat("Grain Amount", &debug.grainAmount, 0.0f, 0.2f);
            ImGui::SliderFloat("Vignette", &debug.grainVignette, 0.0f, 1.0f);
            break;

        default:
            ImGui::Text("No parameters for this effect.");
            break;
    }

    ImGui::Separator();
    ImGui::Text("FBO Preview (raw):");
    ImGui::Image(
        (void*)(intptr_t)fbo_texture,
        ImVec2(400, 300),
        ImVec2(0, 1), ImVec2(1, 0)
    );

    ImGui::End();
}