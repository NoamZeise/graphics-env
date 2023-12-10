#ifndef GAME_HELPER_MOUSE
#define GAME_HELPER_MOUSE

#include <GLFW/glfw3.h>

typedef int MouseButton;

struct MouseState {
    MouseState();
    void mousePosCallback(double x, double y);
    void mouseScrollCallback(double x, double y);
    void mouseButtonCallback(MouseButton btn, int action, int mods);
    double x = 0.0;
    double y = 0.0;
    double scroll = 0.0;
    bool btn[GLFW_MOUSE_BUTTON_LAST + 1];
};

class Mouse {
 public:
    double x();
    double dx();
    double y();
    double dy();
    double scroll();
    bool press(MouseButton btn);
    bool hold(MouseButton btn);

    /// called by glfw/app
    void update();
    void mousePosCallback(double x, double y);
    void mouseScrollCallback(double x, double y);
    void mouseButtonCallback(MouseButton btn, int action, int mods);
 private:
    MouseState state;
    MouseState prevState;
};

#endif
