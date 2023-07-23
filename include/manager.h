#ifndef APP_MANAGER_H
#define APP_MANAGER_H

#include <thread>
#include <atomic>

#include <glm/glm.hpp>

#include <render.h>
#include <graphics/render_config.h>
#include <GameHelper/timer.h>
#include <GameHelper/input.h>
#ifndef NO_AUDIO
#include <audio.h>
#endif

enum class cursorState {
    normal,
    hidden,
    disabled,
};

struct ManagerState {
    int windowWidth = 800;
    int windowHeight = 600;
    bool startFullscreen = false;
    bool fixedWindowRatio = false;
    cursorState cursor = cursorState::normal;
    RenderConfig conf;
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
#ifndef NO_AUDIO
    audio::Manager audio;
#endif
    glm::vec2 screenToRenderSpace(glm::vec2 pos);
    glm::vec2 mousePos();
};

#endif
