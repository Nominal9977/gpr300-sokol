#include "scene.h"

// batteries
#include "batteries/math.h"
#include "batteries/opengl.h"

// ew
#include "ew/procGen.h"

// assimp
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

// imgui
#include "imgui/imgui.h"

// std
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>


// Function pulled and adapted from LearnOpenGl
void TexturedModel::load(const std::string& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        printf("ERROR::ASSIMP::%s\n", importer.GetErrorString());
        return;
    }

    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[i];

        std::vector<Vertex>       vertices;
        std::vector<unsigned int> indices;
        std::string               mat_name;

        // process vertices
        for (unsigned int j = 0; j < mesh->mNumVertices; j++)
        {
            Vertex vertex;

            glm::vec3 vector;
            vector.x         = mesh->mVertices[j].x;
            vector.y         = mesh->mVertices[j].y;
            vector.z         = mesh->mVertices[j].z;
            vertex.Position  = vector;

            if (mesh->HasNormals())
            {
                vector.x        = mesh->mNormals[j].x;
                vector.y        = mesh->mNormals[j].y;
                vector.z        = mesh->mNormals[j].z;
                vertex.Normal   = vector;
            }

            if (mesh->mTextureCoords[0])
            {
                glm::vec2 vec;
                vec.x            = mesh->mTextureCoords[0][j].x;
                vec.y            = mesh->mTextureCoords[0][j].y;
                vertex.TexCoords = vec;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }

        // process indices
        for (unsigned int j = 0; j < mesh->mNumFaces; j++)
        {
            aiFace face = mesh->mFaces[j];
            for (unsigned int k = 0; k < face.mNumIndices; k++)
                indices.push_back(face.mIndices[k]);
        }

        // process material
        if (mesh->mMaterialIndex >= 0)
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
            aiString name;
            material->Get(AI_MATKEY_NAME, name);
            mat_name = name.C_Str();
        }

        // setup GPU buffers
        Submesh sm;
        sm.material_name = mat_name;
        sm.index_count   = (int)indices.size();

        glGenVertexArrays(1, &sm.vao);
        glGenBuffers(1, &sm.vbo);
        glGenBuffers(1, &sm.ebo);

        glBindVertexArray(sm.vao);

        glBindBuffer(GL_ARRAY_BUFFER, sm.vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sm.ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

        // vertex positions
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

        glBindVertexArray(0);
        submeshes.push_back(sm);
    }
}

void TexturedModel::addTexture(const std::string& mat_name, const std::string& tex_path)
{
    textures[mat_name] = std::make_unique<ew::Texture>(tex_path);
}

void TexturedModel::draw(ew::Shader* shader) const
{
    for (const auto& sm : submeshes)
    {
        // bind textures
        glActiveTexture(GL_TEXTURE0);
        auto it = textures.find(sm.material_name);
        if (it != textures.end())
        {
            glBindTexture(GL_TEXTURE_2D, it->second->getID());
            shader->setInt("albedo_tex", 0);
            shader->setInt("has_texture", 1);
        }
        else
        {
            shader->setInt("has_texture", 0);
        }

        // draw mesh
        glBindVertexArray(sm.vao);
        glDrawElements(GL_TRIANGLES, sm.index_count, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    // reset active texture
    glActiveTexture(GL_TEXTURE0);
}

constexpr int kFramebufferWidth = 800;
constexpr int kFramebufferHeight = 600;
constexpr float orbit_radius = 2.0f;


struct FullscreenQuad
{
    GLuint vao;
    GLuint vbo;

    void Initialize()
    {
        // clang-format off
        float quad_vertices[] = {
            // pos (x, y) texcoord (u, v)
            -1.0f,  1.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
             1.0f, -1.0f, 1.0f, 0.0f,

            -1.0f,  1.0f, 0.0f, 1.0f,
             1.0f, -1.0f, 1.0f, 0.0f,
             1.0f,  1.0f, 1.0f, 1.0f,
        };
        // clang-format on

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(sizeof(float) * 2));

        glBindVertexArray(0);
    }
} fullscreen_quad;

struct Framebuffer
{
    GLuint fbo;
    GLuint position;
    GLuint normal;
    GLuint albedo;
    GLuint material;
    GLuint depth;

    void Initialize()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        // position attachment
        glGenTextures(1, &position);
        glBindTexture(GL_TEXTURE_2D, position);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position, 0);

        // normal attachment
        glGenTextures(1, &normal);
        glBindTexture(GL_TEXTURE_2D, normal);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal, 0);

        // albedo attachment
        glGenTextures(1, &albedo);
        glBindTexture(GL_TEXTURE_2D, albedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedo, 0);

        // material attachment
        glGenTextures(1, &material);
        glBindTexture(GL_TEXTURE_2D, material);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, material, 0);

        // depth attachment
        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kFramebufferWidth, kFramebufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

        GLenum array[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
        glDrawBuffers(4, array);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            printf("Framebuffer incomplete\n");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} framebuffer;

struct LighVolumebuffer
{
    GLuint fbo;
    GLuint color;
    GLuint depth;

    void Initialize()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &color);
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kFramebufferWidth, kFramebufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            printf("Light volume buffer incomplete\n");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} lightvolumebuffer;

struct WaterBuffer
{
    GLuint fbo;
    GLuint color;
    GLuint depth;

    void Initialize()
    {
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        glGenTextures(1, &color);
        glBindTexture(GL_TEXTURE_2D, color);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, kFramebufferWidth, kFramebufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);

        glGenTextures(1, &depth);
        glBindTexture(GL_TEXTURE_2D, depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, kFramebufferWidth, kFramebufferHeight, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            printf("Water buffer incomplete\n");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
} reflectionbuffer, refractionbuffer;


struct Material
{
    float ambient = 1.0f;
    float diffuse = 0.5f;
    float specular = 0.5f;
    float shininess = 0.5f;
} material;

struct
{
    int width = 0;
    float light_radius = 2.5f;
    bool draw_light_volume = false;
} debug;

Scene::Scene()
{
    suzanne = std::make_unique<ew::Model>("assets/models/suzanne.obj");
    Land = std::make_unique<ew::Model>("assets/models/landscape.obj");
    tree = std::make_unique<ew::Model>("assets/models/plant_pine_tree.obj");

    // LOAD DE HOUSEEEE (fuck multi-texturing)
    house_model.load("assets/models/house_cabin_1256.obj");
    {
        const std::string source = "assets/shaders/house_cabin/";
        house_model.addTexture("house_cabin_1256_grey_MatSG", source + "house_cabin_1256_grey_Mat_baseColor.png");
        house_model.addTexture("house_cabin_1256_glass_MatSG", source + "house_cabin_1256_glass_Mat_baseColor.png");
        house_model.addTexture("house_cabin_1256_brown_wood_MatSG", source + "house_cabin_1256_brown_wood_Mat_baseColor.png");
        house_model.addTexture("house_cabin_1256_Green_MatSG", source + "house_cabin_1256_Green_Mat_baseColor.png");
        house_model.addTexture("house_cabin_1256_cream_MatSG", source + "house_cabin_1256_cream_Mat_baseColor.png");
        house_model.addTexture("house_cabin_1256_roofGrey_MatSG", source + "house_cabin_1256_roofGrey_Mat_baseColor.png");
    }


    // Setup The Shaders
    geometry = std::make_unique<ew::Shader>("assets/shaders/Final_Project/geometry.vs", "assets/shaders/Final_Project/geometry.fs");
    water = std::make_unique<ew::Shader>("assets/shaders/Final_Project/water.vs", "assets/shaders/Final_Project/water.fs");
    reflection_shader = std::make_unique<ew::Shader>("assets/shaders/Final_Project/geometry.vs", "assets/shaders/Final_Project/reflection.fs");
    blinnphong = std::make_unique<ew::Shader>("assets/shaders/Final_Project/blinnphong.vs", "assets/shaders/Final_Project/blinnphong.fs");
    fullScreen = std::make_unique<ew::Shader>("assets/shaders/Final_Project/fullscreen.vs", "assets/shaders/Final_Project/fullscreen.fs");
    plane.load(ew::createPlane(60.0f, 60.0f, 1));
    wave_tex  = std::make_unique<ew::Texture>("assets/doubledash/wave_tex.png");
    wave_warp = std::make_unique<ew::Texture>("assets/doubledash/wave_warp.png");
    tree_tex = std::make_unique<ew::Texture>("assets/shaders/plant_pine_tree/plant_pine_tree_abstract_baseColor.png");
    land_tex = std::make_unique<ew::Texture>("assets/textures/Brick.png");

    ambient = {
        .intensity = 1.0f,
        .color = {0.5f, 0.5f, 0.5f},
    };

    // Initialize Buffers
    framebuffer.Initialize();
    lightvolumebuffer.Initialize();
    reflectionbuffer.Initialize();
    refractionbuffer.Initialize();
    fullscreen_quad.Initialize();

    InitializeInstanceData();
}

Scene::~Scene()
{
}

void Scene::InitializeInstanceData(void)
{
    auto width = debug.width;
    auto size = (width - (-width) + 1) * (width - (-width) + 1);
    light_instances.resize(size);

    auto i = 0;
    for (auto x = -debug.width; x <= debug.width; x++)
    {
        for (auto y = -debug.width; y <= debug.width; y++, i++)
        {
            const auto position = glm::vec3(x * 3.0f, 0, y * 3.0f);
            const auto orbit = batteries::random_point_on_sphere();

            light_instances[i] = {
                .brightness = (rand() % 80 + 20) * 0.1f,
                .color = batteries::random_color(),
                .position = glm::vec4(position, 1.0f) + orbit * orbit_radius,
            };
        }
    }
}

void Scene::Update(float dt)
{
    batteries::Scene::Update(dt);
}

void Scene::Render(void)
{
    const auto view_proj = camera.Projection() * camera.View();

    const glm::mat4 land_model  = glm::scale(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f)), glm::vec3(3.0f));
    const glm::mat4 plane_model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, water_y, 0.0f));

    //sun direction
    const float az = glm::radians(sun_rotiaon);
    const float el = glm::radians(sun_elevation);

    const glm::vec3 sun_dir = glm::normalize(glm::vec3(
        glm::cos(el) * glm::sin(az),
        glm::sin(el),
        glm::cos(el) * glm::cos(az)
    ));

    // blend to sky
    constexpr float sky_r = 0.53f, sky_g = 0.81f, sky_b = 0.98f;

    // Compute reflected camera
    glm::mat4 ref_view_proj;
    {
        glm::mat4 view = camera.View();

        glm::vec3 cam_up = glm::vec3(view[0][1], view[1][1], view[2][1]);

        glm::vec3 ref_pos = {camera.position.x, 2.0f * water_y - camera.position.y, camera.position.z};
        glm::vec3 forward = -glm::vec3(view[0][2], view[1][2], view[2][2]);
        glm::vec3 ref_fwd = {forward.x, -forward.y, forward.z};

        glm::vec3 ref_up  = {cam_up.x, -cam_up.y, cam_up.z};

        ref_view_proj = camera.Projection() *
            glm::lookAt(ref_pos, ref_pos + ref_fwd, ref_up);
    }

    //Reflection Pass
    {
        glBindFramebuffer(GL_FRAMEBUFFER, reflectionbuffer.fbo);
        glEnable(GL_CLIP_DISTANCE0);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        //uses sky color so empty regions blend naturally (why didnt we start with this)
        glClearColor(sky_r, sky_g, sky_b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        reflection_shader->use();
        reflection_shader->setInt("albedo_tex", 0);
        reflection_shader->setMat4("view_proj", ref_view_proj);
        reflection_shader->setVec4("clip_plane", glm::vec4(0.0f, 1.0f, 0.0f, -water_y + 0.1f));
        reflection_shader->setVec3("sun_dir", sun_dir);
        reflection_shader->setVec3("sun_color", sun_color);
        reflection_shader->setFloat("sun_intensity", sun_intensity);
        reflection_shader->setInt("has_texture", 0);
        reflection_shader->setMat4("model", glm::translate(glm::mat4(1.0f), suzanne_pos));
        suzanne->draw();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, land_tex->getID());
        reflection_shader->setInt("has_texture", 1);
        reflection_shader->setMat4("model", land_model);
        Land->draw();

        reflection_shader->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), house_pos), glm::vec3(house_scale)));
        house_model.draw(reflection_shader.get());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tree_tex->getID());
        reflection_shader->setInt("has_texture", 1);
        for (const auto& pos : tree_positions) {
            reflection_shader->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), pos), glm::vec3(tree_scale)));
            tree->draw();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, refractionbuffer.fbo);
        glEnable(GL_CLIP_DISTANCE0);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(sky_r, sky_g, sky_b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        reflection_shader->use();
        reflection_shader->setInt("albedo_tex", 0);
        reflection_shader->setMat4("view_proj", view_proj);
        reflection_shader->setVec4("clip_plane", glm::vec4(0.0f, -1.0f, 0.0f, water_y + 0.1f));
        reflection_shader->setVec3("sun_dir", sun_dir);
        reflection_shader->setVec3("sun_color", sun_color);
        reflection_shader->setFloat("sun_intensity", sun_intensity);
        reflection_shader->setInt("has_texture", 0);
        reflection_shader->setMat4("model", glm::translate(glm::mat4(1.0f), suzanne_pos));
        suzanne->draw();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, land_tex->getID());
        reflection_shader->setInt("has_texture", 1);
        reflection_shader->setMat4("model", land_model);
        Land->draw();

        reflection_shader->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), house_pos), glm::vec3(house_scale)));
        house_model.draw(reflection_shader.get());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tree_tex->getID());
        reflection_shader->setInt("has_texture", 1);
        for (const auto& pos : tree_positions) {
            reflection_shader->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), pos), glm::vec3(tree_scale)));
            tree->draw();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    glDisable(GL_CLIP_DISTANCE0);

    //Full Water Pass
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.fbo);
    {
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        geometry->use();
        geometry->setMat4("view_proj", view_proj);
        geometry->setVec4("clip_plane", glm::vec4(0.0f, 1.0f, 0.0f, 99999.0f));
        geometry->setFloat("material.ambient", material.ambient);
        geometry->setFloat("material.diffuse", material.diffuse);
        geometry->setFloat("material.specular", material.specular);
        geometry->setFloat("material.shininess", material.shininess);
        geometry->setInt("albedo_tex", 0);

        geometry->setInt("has_texture", 0);
        geometry->setMat4("model", glm::translate(glm::mat4(1.0f), suzanne_pos));
        suzanne->draw();

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, land_tex->getID());

        geometry->setInt("has_texture", 1);
        geometry->setMat4("model", land_model);
        Land->draw();

        geometry->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), house_pos), glm::vec3(house_scale)));
        house_model.draw(geometry.get());

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, tree_tex->getID());
        geometry->setInt("has_texture", 1);
        for (const auto& pos : tree_positions) {
            geometry->setMat4("model", glm::scale(glm::translate(glm::mat4(1.0f), pos), glm::vec3(tree_scale)));
            tree->draw();
        }

        // Water plane
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, reflectionbuffer.color);
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, refractionbuffer.color);
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_2D, refractionbuffer.depth);
        glActiveTexture(GL_TEXTURE7);
        glBindTexture(GL_TEXTURE_2D, wave_tex->getID());
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, wave_warp->getID());

        water->use();
        water->setMat4("view_proj", view_proj);
        water->setMat4("ref_view_proj", ref_view_proj);
        water->setVec3("camera_position", camera.position);
        water->setVec2("screen_size", glm::vec2(kFramebufferWidth, kFramebufferHeight));
        water->setInt("reflection_map", 4);
        water->setInt("refraction_map", 5);
        water->setInt("refraction_depth", 6);
        water->setInt("wave_tex", 7);
        water->setInt("wave_warp", 8);
        water->setFloat("time", (float)time.absolute);
        water->setVec3("fog_color", fog_color);
        water->setFloat("fog_max_depth", fog_max_depth);
        water->setFloat("fresnel_F0", fresnel);
        water->setVec3("sun_dir", sun_dir);
        water->setVec3("sun_color", sun_color);
        water->setFloat("sun_intensity", sun_intensity);
        water->setFloat("near_plane", 0.01f);
        water->setFloat("far_plane", 1000.0f);
        water->setFloat("material.ambient", 0.3f);
        water->setFloat("material.diffuse", 0.6f);
        water->setFloat("material.specular", 0.9f);
        water->setFloat("material.shininess", 0.9f);
        water->setMat4("model", plane_model);
        plane.draw();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //Light pass 
    glBindFramebuffer(GL_FRAMEBUFFER, lightvolumebuffer.fbo);
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, framebuffer.position);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffer.normal);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, framebuffer.albedo);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, framebuffer.material);

        blinnphong->use();
        blinnphong->setVec3("camera_position", camera.position);
        blinnphong->setInt("g_position", 0);
        blinnphong->setInt("g_normal", 1);
        blinnphong->setInt("g_albedo", 2);
        blinnphong->setInt("g_material", 3);

        glBindVertexArray(fullscreen_quad.vao);
        for (const auto& l : light_instances)
        {
            blinnphong->setVec3("light.position", l.position);
            blinnphong->setVec3("light.color", l.color);
            blinnphong->setFloat("light.radius", l.brightness * debug.light_radius);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        glDisable(GL_BLEND);
    }
    //FullScreen Pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    {
        fullScreen->use();
        fullScreen->setInt("screen", 0);
        fullScreen->setInt("g_albedo", 1);
        fullScreen->setInt("g_normal", 2);
        fullScreen->setVec3("ambient_color", ambient.color);
        fullScreen->setFloat("ambient_strength", ambient.intensity);
        fullScreen->setVec3("sky_color", glm::vec3(sky_r, sky_g, sky_b));
        fullScreen->setVec3("sun_dir", sun_dir);
        fullScreen->setVec3("sun_color", sun_color);
        fullScreen->setFloat("sun_intensity", sun_intensity);

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glBindVertexArray(fullscreen_quad.vao);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, lightvolumebuffer.color);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, framebuffer.albedo);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, framebuffer.normal);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glBlitFramebuffer(0, 0, kFramebufferWidth, kFramebufferHeight,
                          0, 0, kFramebufferWidth, kFramebufferHeight,
                          GL_DEPTH_BUFFER_BIT, GL_NEAREST);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void Scene::Debug(void)
{
    cameracontroller.Debug();

    ImGui::Begin("Controlls", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::Checkbox("Paused", &time.paused);
    ImGui::SliderFloat("Time Factor", &time.factor, 0.0f, 10.0f);
    ImGui::DragFloat3("Suzanne", &suzanne_pos.x, 0.1f);

    if (ImGui::CollapsingHeader("Water"))
    {
        ImGui::SliderFloat("Water hight",    &water_y,       -10.0f, 10.0f);
        ImGui::ColorEdit3("Fog Color",  &fog_color.r);
        ImGui::SliderFloat("Fog Depth", &fog_max_depth,   0.1f, 20.0f);
        ImGui::Separator();
        ImGui::SliderFloat("Reflectiveness", &fresnel, 0.0f, 1.0f);
    }

    if (ImGui::CollapsingHeader("Sun"))
    {
        ImGui::SliderFloat("Rotation",   &sun_rotiaon,   0.0f, 360.0f);
        ImGui::SliderFloat("Sun hight",     &sun_elevation, 0.0f,  90.0f);
        ImGui::ColorEdit3("Color",       &sun_color.r);
        ImGui::SliderFloat("Brightness", &sun_intensity, 0.0f,   2.0f);
    }

    if (ImGui::CollapsingHeader("Geometry Buffer"))
    {
        ImVec2 uv_min(0.0f, 1.0f);
        ImVec2 uv_max(1.0f, 0.0f);

        ImGui::Text("Lighting:");
        ImGui::Image((ImTextureID)(intptr_t)lightvolumebuffer.color, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Albedo:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.albedo, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Material:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.material, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Position:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.position, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Normal:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.normal, ImVec2(200, 150), uv_min, uv_max);

        ImGui::Text("Depth:");
        ImGui::Image((ImTextureID)(intptr_t)framebuffer.depth, ImVec2(200, 150), uv_min, uv_max);
    }

    ImGui::End();
}