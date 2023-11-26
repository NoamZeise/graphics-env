#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <thread>
#include <atomic>
#include <string>

#include <graphics/render.h>
#include <GameHelper/timer.h>
#include <GameHelper/input.h>
#include <glm/glm.hpp>
#ifndef NO_AUDIO
#include <audio.h>
#endif

enum class cursorState {
  normal,
  hidden,
  disabled,
};

enum class RenderFramework {
   Vulkan,
   OpenGL,
};

struct ManagerState {
    int windowWidth = 800;
    int windowHeight = 600;
    std::string windowTitle = "App";
    bool startFullscreen = false;
    bool fixedWindowRatio = false;
    cursorState cursor = cursorState::normal;
    bool hideWindowOnCreate = false;
    RenderFramework defaultRenderer = RenderFramework::Vulkan;
    RenderConfig conf;
};

struct Manager {
    Manager(ManagerState state);
    ~Manager();
    void update();
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();
    void setWindowSize(int width, int height, bool updateGlfw);
    void setWindowSize(int width, int height);
    glm::vec2 winSize();
    bool winActive();
    RenderFramework backend();
    
    GLFWwindow *window;
    Render* render;
    gamehelper::Timer timer;
    gamehelper::Input input;
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
