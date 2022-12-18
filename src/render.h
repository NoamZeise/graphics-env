
//namespace ogl {
//#include "opengl-render/render.h"
//#include "opengl-render/resources/resources.h"
//} //namespace OGL

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <audio.h>
#include <input.h>
#include <timer.h>
#include <glmhelper.h>

#include "vulkan-render/render.h"
#include "vulkan-render/resources/resources.h"


class Render {
public:
  Render(GLFWwindow *window)
  {
    Render(window, glm::vec2(600, 600));
  }
  Render(GLFWwindow *window, glm::vec2 target)
  {
    //in future, test for vulkan presence
    usingVulkan = true;

    if(usingVulkan)
      mvkRender = new mvk::Render(window, target);
  }
  ~Render();
  static void SetGLFWWindowHints() {
    //need to have this run before Create Window in app (maybe run here?)

    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  }
  void set3DViewMatrixAndFov(glm::mat4 view, float fov, glm::vec4 camPos) {
    if(usingVulkan)
      mvkRender->set3DViewMatrixAndFov(view, fov, camPos);
  }
  void set2DViewMatrixAndScale(glm::mat4 view, float scale);
  void setLightDirection(glm::vec4 lightDir);
  void restartResourceLoad();

  Resource::Texture LoadTexture(std::string filepath) {
    if(usingVulkan)
      mvkRender->LoadTexture(filepath);
  }
  Resource::Font LoadFont(std::string filepath) {
    if(usingVulkan)
      return mvkRender->LoadFont(filepath);
  }
  Resource::Model LoadModel(std::string filepath);
  Resource::Model LoadAnimatedModel(std::string filepath, std::vector<Resource::ModelAnimation> *pGetAnimations);

  void EndResourceLoad();

  void Begin3DDraw();
  void BeginAnim3DDraw();
  void Begin2DDraw();
  void DrawModel(    Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix);
  void DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix, Resource::ModelAnimation *animation);
  void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour, glm::vec4 texOffset);
  void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour);
  void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix);
  void DrawString(Resource::Font font, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour, float rotate);
  void DrawString(Resource::Font font, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour);
  float MeasureString(Resource::Font font, std::string text, float size);
  void EndDraw(std::atomic<bool> &submit);

  void FramebufferResize();

 private:
  bool usingVulkan = true;
  mvk::Render* mvkRender;
};
