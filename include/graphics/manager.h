#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <thread>
#include <atomic>
#include <string>

#include <graphics/render.h>
#include <game/timer.h>
#include <game/input.h>
#include <glm/glm.hpp>
#ifndef NO_AUDIO
#include <audio.h>
#endif

enum class cursorState {
  normal,
  hidden,
  disabled,
};

const int RENDER_FRAMEWORK_COUNT = 2;
enum class RenderFramework {
   Vulkan = 0,
   OpenGL,
};

struct ManagerState {
    int windowWidth = 800;
    int windowHeight = 600;
    std::string windowTitle = "graphics env program";
    bool startFullscreen = false;
    bool fixedWindowRatio = false;
    bool hideWindowOnCreate = false;
    cursorState cursor = cursorState::normal;
    RenderFramework defaultRenderer = RenderFramework::Vulkan;
    RenderConfig render;
    
    void setShader(RenderFramework framework,
		   shader::pipeline pipeline, shader::stage stage,
		   std::string path);
    void setShader(shader::pipeline pipeline,
		   shader::stage stage,
		   std::string vulkan_path,
		   std::string ogl_path);
    shader::PipelineSetup getPipelineSetup(RenderFramework framework);
private:
    shader::PipelineSetup pipeline[RENDER_FRAMEWORK_COUNT];
};

struct Manager {
    Manager(ManagerState state);
    ~Manager();
    void update();
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();
    void setWindowSize(int width, int height);
    /// used by glfw callback to set the window size values
    /// when the user changes the size of the window.
    void _windowSizeCallback(int width, int height);
    glm::vec2 winSize();
    bool winActive();
    RenderFramework backend();
    
    GLFWwindow *window;
    Render* render;
    Timer timer;
    Input input;
    float fov = 45.0f;
    float scale2d = 1.0f;
#ifndef NO_AUDIO
    audio::Manager audio;
#endif
    glm::vec2 screenToRenderSpace(glm::vec2 pos);
    glm::vec2 mousePos();
private:
    RenderFramework framework;
    int winWidth;
    int winHeight;
};

#endif
