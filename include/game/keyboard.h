#ifndef GAME_HELPER_KEYBOARD_H
#define GAME_HELPER_KEYBOARD_H

#include <GLFW/glfw3.h>

/// From 0 to GLFW_KEY_LAST
typedef int InputKey;

struct KeyboardState {
    KeyboardState();
    void handleKey(InputKey key, int scancode, int action);
    bool key[GLFW_KEY_LAST + 1];
};

struct Keyboard {
public:
    /// Get whether the key has just been pressed
    /// On Release Builds, Will return false on invalid key and log an error
    /// On Debug, will throw an exception
    bool press(InputKey key);
    bool hold(InputKey key);
    
    /// Called by glfw/app to update key state
    void update();
    void handleKey(InputKey key, int scancode, int action);
    
 private:
    KeyboardState state;
    KeyboardState prevState;
};

#endif
