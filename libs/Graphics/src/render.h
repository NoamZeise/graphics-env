#ifndef GRAPHICS_ENV_RENDERER
#define GRAPHICS_ENV_RENDERER

#include <resources/resources.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <atomic>

namespace vkenv{
    class Render;
}
namespace glenv {
    class GLRender;
}

/*
  A wrapper around OpenGL-Env and Vulkan-Env Render.
  These two share the same public interface, so this class
  will call whichever is selected as the RenderFramework
 */

enum class RenderFramework {
   VULKAN,
   OPENGL,
};

#define pFrameworkSwitch(render, vk, gl) switch(render->getRenderFramework()) { \
   case RenderFramework::VULKAN: vk; break;    \
        case RenderFramework::OPENGL: gl; break;    \
}

class Render {
public:
  Render(RenderFramework preferredRenderer);
  ~Render();
  bool NoApiLoaded() { return noApiLoaded; }
  void LoadRender(GLFWwindow *window);
    void LoadRender(GLFWwindow *window, glm::vec2 target);

    Resource::Texture LoadTexture(std::string filepath);
    Resource::Font LoadFont(std::string filepath);
    Resource::Model LoadModel(std::string filepath);
    Resource::Model LoadAnimatedModel(std::string filepath, std::vector<Resource::ModelAnimation> *pGetAnimations);
	
    void LoadResourcesToGPU();
    void UseLoadedResources();

    void Begin3DDraw();
    void BeginAnim3DDraw() ;
    void Begin2DDraw();
  void DrawModel(Resource::Model model, glm::mat4 modelMatrix,
                 glm::mat4 normalMatrix);
  void DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix,
                     glm::mat4 normalMatrix,
                     Resource::ModelAnimation *animation);
  void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix,
                glm::vec4 colour, glm::vec4 texOffset);
  void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix,
                glm::vec4 colour);
    void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix);
  void DrawString(Resource::Font font, std::string text, glm::vec2 position,
                  float size, float depth, glm::vec4 colour, float rotate);
  void DrawString(Resource::Font font, std::string text, glm::vec2 position,
                  float size, float depth, glm::vec4 colour);
    float MeasureString(Resource::Font font, std::string text, float size);
    void EndDraw(std::atomic<bool> &submit);

    void FramebufferResize();

    void set3DViewMatrixAndFov(glm::mat4 view, float fov, glm::vec4 camPos);
    void set2DViewMatrixAndScale(glm::mat4 view, float scale);
    void setLightDirection(glm::vec4 lightDir);
    void setForceTargetRes(bool force); 
    bool isTargetResForced();
    void setTargetResolution(glm::vec2 resolution);
    glm::vec2 getTargetResolution();
    void setVsync(bool vsync);

  RenderFramework getRenderFramework() {
    return renderer;
  }

private:
  RenderFramework renderer;
  bool noApiLoaded = false;
  vkenv::Render *vkRender;
  glenv::GLRender *glRender;
};

#endif
