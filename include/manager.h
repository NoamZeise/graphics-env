#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <thread>
#include <atomic>

#include <glm/glm.hpp>

#include <render.h>
#include <GameHelper/timer.h>
#include <GameHelper/input.h>
#include <audio.h>

enum class cursorState {
    normal,
    hidden,
    disabled,
};

struct ManagerState {
    int windowWidth = 800;
    int windowHeight = 600;
    // Zero to use window resolution
    int framebufferWidth = 0;
    int framebufferHeight = 0;
    bool startFullscreen = false;
    bool fixedWindowRatio = false;
    cursorState cursor = cursorState::normal;
};

struct Manager {
    Manager(RenderFramework renderer,
	    ManagerState state);
    ~Manager();
    void update();
    void setFullscreen(bool fullscreen);
    void toggleFullscreen();
    
    GLFWwindow *window;
    Render* render;
    int winWidth;
    int winHeight;
    gamehelper::Timer timer;
    gamehelper::Input input;
    audio::Manager audio;

    glm::vec2 screenToRenderSpace(glm::vec2 pos);
    glm::vec2 mousePos();
};

#endif
