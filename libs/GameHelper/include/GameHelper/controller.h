#ifndef GAME_HELPER_CONTROLLER_H
#define GAME_HELPER_CONTROLLER_H

#include <GLFW/glfw3.h>
#include <vector>

struct ControllerHolder;

class ControllerManager {
 public:
    ControllerManager();
    ~ControllerManager();
    void init();
    void update();

    bool connected(int controller);
    bool hold(int controller, int btn);
    bool press(int controller, int btn);
    float axis(int controller, int axis);
 private:
    ControllerHolder* controllers;
    bool initialized = false;
};

#endif
