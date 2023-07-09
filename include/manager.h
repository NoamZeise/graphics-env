#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <thread>
#include <atomic>
#include <string>

#include <glm/glm.hpp>

#include <render.h>
#include <GameHelper/timer.h>
#include <GameHelper/input.h>

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
    std::string windowName = "App";
};

struct Manager {
    Manager(ManagerState state);
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
    glm::vec2 correctedMouse;
    gamehelper::Timer timer;
    gamehelper::Input input;

    glm::vec2 screenToRenderSpace(glm::vec2 pos);
    glm::vec2 mousePos();

private:
    void init(RenderFramework renderer, ManagerState state);
};

#endif
