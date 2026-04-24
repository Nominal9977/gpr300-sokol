#pragma once

// batteries
#include "batteries/scene.h"
#include "batteries/lights.h"
#include "batteries/opengl.h"

// ew
#include "ew/model.h"
#include "ew/shader.h"
#include "ew/texture.h"

class Scene final : public batteries::Scene
{
  public:
    Scene();
    virtual ~Scene();

    void Update(float dt);
    void Render(void);
    void Debug(void);

  private:

    void createDepthBuffer();

    void createFrameBuffer();

    std::unique_ptr<ew::Model>  suzanne;
    std::unique_ptr<ew::Shader> toonShader;
        std::unique_ptr<ew::Shader> depth;
    std::unique_ptr<ew::Texture> texture;



    std::vector<std::unique_ptr<ew::Shader>> mPostShaders;
    int mSelectedPost = 0;

    batteries::light_t light;
    int mSelectedShader = 0;
    
    struct {
      glm::vec3 color1;
      glm::vec3 color2;
    } pallet;

    ew::Mesh plane;

    GLuint fbo;
    GLuint fbo_texture;
    GLuint fbo_depth;

    //depth buffer
    GLuint Shadow_fbo;
    //GLuint fbo_texture;
    GLuint shadow_depth;




};
