#include <game/mouse.h>

#ifdef NDEBUG
#include <iostream>
#else
#include <stdexcept>
#endif

//#include <iostream>

void MouseState::mousePosCallback(double x, double y) {
    this->x = x;
    this->y = y;
    if(uninitialised)
	uninitialised = callbackCount++ > 1;
}

void MouseState::mouseScrollCallback(double x, double y) {
    this->scroll = y;
}

void MouseState::mouseButtonCallback(MouseButton btn, int action, int mods) {
    if(btn >= 0 && btn <= GLFW_MOUSE_BUTTON_LAST) {
	if(action == GLFW_PRESS) {
	    this->btn[btn] = true;
	} else if(action == GLFW_RELEASE) {
	    this->btn[btn] = false;
	}
    }
}

MouseState::MouseState() {
    for(int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++)
	this->btn[i] = false;
}

double Mouse::x() { return this->state.x; }

double Mouse::dx() {
    if(this->prevState.uninitialised)
	return 0;
    return this->state.x - this->prevState.x;
}

double Mouse::dy() {
    if(this->prevState.uninitialised)
	return 0;
    return this->state.y - this->prevState.y;
}

double Mouse::y() { return this->state.y; }

double Mouse::scroll() { return this->state.scroll; }

bool Mouse::hold(MouseButton btn) {
    if (btn < 0 || btn > GLFW_MOUSE_BUTTON_LAST) {      
#ifdef NDEBUG
	std::cerr << "Error: Mouse Btn Key Code out of range, returning false\n";       
	return false;                                                              
#else
	throw std::runtime_error("Mouse Btn Key Code was out of range");
#endif
    }
    return this->state.btn[btn];
}

bool Mouse::press(MouseButton btn) {
    if(this->prevState.uninitialised)
	return false;
    return this->hold(btn) && !this->prevState.btn[btn];
}

void Mouse::mousePosCallback(double x, double y) {
    this->state.mousePosCallback(x, y);
}

void Mouse::mouseScrollCallback(double x, double y) {
    this->state.mouseScrollCallback(x, y);
}

void Mouse::mouseButtonCallback(MouseButton btn, int action, int mods) {
    this->state.mouseButtonCallback(btn, action, mods);
}

void Mouse::update() {
    this->prevState = this->state;
    this->state.scroll = 0.0;
}

void Mouse::reset() {
    this->state.uninitialised = true;
    this->state.callbackCount = 0;
    this->prevState = state;
}
