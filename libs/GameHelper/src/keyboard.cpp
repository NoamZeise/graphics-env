#include <GameHelper/keyboard.h>
#include <iostream>

#ifndef NDEBUG
#include <stdexcept>
#endif

void KeyboardState::handleKey(InputKey key, int scancode, int action) {
    if(key >= 0 && key <=GLFW_KEY_LAST) {
	if(action == GLFW_PRESS) {
	    this->key[key] = true;
	} else if(action == GLFW_RELEASE) {
	    this->key[key] = false;
	}
    }
}

KeyboardState::KeyboardState() {
    for(int i = 0; i <= GLFW_KEY_LAST; i++)
	this->key[i] = false;
}

bool Keyboard::hold(InputKey key) {
  if (key < 0 || key > GLFW_KEY_LAST) {
    #ifdef NDEBUG
      std::cerr << "Error: Keyboard Key Code out of range, returning false\n"; 
      return false;                                                              
#else
      throw std::runtime_error("Keyboard Key Code was out of range");
#endif
  }
  return this->state.key[key];
}

bool Keyboard::press(InputKey key) {
    return this->hold(key) && !this->prevState.key[key];
}


void Keyboard::update() {
    this->prevState = this->state;
}

void Keyboard::handleKey(InputKey key, int scancode, int action) {
    this->state.handleKey(key, scancode, action);
}
