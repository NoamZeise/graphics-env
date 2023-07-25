#include <GameHelper/controller.h>

#include <iostream>

bool controllerStateChanged = false;
bool controllersConnected[GLFW_JOYSTICK_LAST + 1];

struct ControllerState {
    ControllerState();
    bool button[GLFW_GAMEPAD_BUTTON_LAST + 1];
    float axis[GLFW_GAMEPAD_AXIS_LAST + 1];
};

struct BothControllerStates {
    BothControllerStates(int jid) {
	this->ID = jid;
    }
    bool hold(int btn) {
	if(btn < 0 || btn > GLFW_GAMEPAD_BUTTON_LAST) {
	    std::cerr << "GamePad button out of range, returning false\n";
	    return false;
	}
	return current.button[btn];
    }
    bool press(int btn) {
	if(btn < 0 || btn > GLFW_GAMEPAD_BUTTON_LAST) {
	    std::cerr << "GamePad button out of range, returning false\n";
	    return false;
	}
	return hold(btn) && !prev.button[btn];
    }
    float axis(int axis) {
	if(axis < 0 || axis > GLFW_GAMEPAD_AXIS_LAST) {
	    std::cerr << "GamePad axis out of range, returning 0\n";
	    return 0.0;
	}
	return current.axis[axis];
    }
    int ID;
    ControllerState current;
    ControllerState prev;
};

struct ControllerHolder {
    std::vector<BothControllerStates> c;
};

ControllerState::ControllerState() {
    for(int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++)
	this->button[i] = false;
    for(int i = 0; i < GLFW_GAMEPAD_AXIS_LAST; i++)
	this->axis[i] = 0.0;
}

static void joyCallback(int jid, int event) {
    if(event == GLFW_CONNECTED && glfwJoystickIsGamepad(jid)) {
	std::cout << "controller connected id: " << jid << std::endl;
	controllersConnected[jid] = true;
    }
    if(event == GLFW_DISCONNECTED && controllersConnected[jid]) {
	std::cout << "controller disconnected id: " << jid << std::endl;
	controllerStateChanged = true;
	controllersConnected[jid] = false;
    }
}

ControllerManager::ControllerManager() {
    controllers = new ControllerHolder();
    for(int i = 0; i < GLFW_JOYSTICK_LAST; i++)
	controllersConnected[i] = false;
}

ControllerManager::~ControllerManager() {
    delete controllers;
}

void ControllerManager::init() {
    glfwSetJoystickCallback(joyCallback);
    for(int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
	glfwSetJoystickUserPointer(i, this);
	if(glfwJoystickIsGamepad(i)) {
	    controllerStateChanged = true;
	    controllersConnected[i] = true;
	}
    }
    initialized = true;
}

void ControllerManager::update() {
    if(!initialized)
	std::cerr << "Tried to update controller manager before initializing!\n";
    if(controllerStateChanged) {
	bool unchecked[GLFW_JOYSTICK_LAST + 1];
	for(int i = 0; i <= GLFW_JOYSTICK_LAST; i++)
	    unchecked[i] = controllersConnected[i];
	for(int i = 0; i < controllers->c.size(); i++) {
	    if(!controllersConnected[controllers->c[i].ID])
		controllers->c.erase(controllers->c.begin() + i--);
	    else
		unchecked[controllers->c[i].ID] = false;
	}
	for(int i = 0; i <= GLFW_JOYSTICK_LAST; i++)
	    if(unchecked[i])
		controllers->c.push_back(BothControllerStates(i));
	controllerStateChanged = false;
    }
    for(int i = 0; i < controllers->c.size(); i++) {
	GLFWgamepadstate state;
	if(glfwGetGamepadState(controllers->c[i].ID, &state)) {
	    for(int b = 0; b <= GLFW_GAMEPAD_BUTTON_LAST; b++)
		controllers->c[i].current.button[b] = state.buttons[b];
	    for(int a = 0; a < GLFW_GAMEPAD_AXIS_LAST; a++)
		controllers->c[i].current.axis[a] = state.axes[a];
	    controllers->c[i].prev = controllers->c[i].current;
	}
    }
}

bool ControllerManager::connected(int controller) {
    return controller >= 0 && controller < controllers->c.size();
}

bool ControllerManager::hold(int controller, int btn) {
    if(!connected(controller)) {
	std::cerr << "no controller connected to that slot\n";
	return false;
    }
    return controllers->c[controller].hold(btn);
}

bool ControllerManager::press(int controller, int btn) {
    if(!connected(controller)) {
	std::cerr << "no controller connected to that slot\n";
	return false;
    }
    return controllers->c[controller].press(btn);
}

float ControllerManager::axis(int controller, int axis) {
    if(!connected(controller)) {
	std::cerr << "no controller connected to that slot\n";
	return false;
    }
    return controllers->c[controller].axis(axis);
}
