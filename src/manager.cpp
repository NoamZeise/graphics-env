#include <manager.h>

#include <stdexcept>

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouseBtnCallback(GLFWwindow *window, int button, int action, int mods);
void errorCallback(int error, const char *description);

Manager::Manager(RenderFramework renderer,
		 ManagerState state) {
    winWidth = state.windowWidth;
    winHeight = state.windowHeight;

    glfwSetErrorCallback(errorCallback);
    if(!glfwInit())
	throw std::runtime_error("Failed to initialize GLFW!");
    render = new Render(renderer);
    if(render->NoApiLoaded())
	throw std::runtime_error("Failed to load any graphics apis!");

    window = glfwCreateWindow(winWidth, winHeight, "App",
			      state.startFullscreen ? glfwGetPrimaryMonitor() : NULL,
			      nullptr);
    if(!window)
	throw std::runtime_error("glfw failed to create window!");

    //set glfw callbacks
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetMouseButtonCallback(window, mouseBtnCallback);
    input.c.init();

    int cursor;
    switch(state.cursor) {
    case cursorState::normal:
	cursor = GLFW_CURSOR_NORMAL;
	break;
    case cursorState::disabled:
	cursor = GLFW_CURSOR_DISABLED;
	break;
    case cursorState::hidden:
	cursor = GLFW_CURSOR_HIDDEN;
	break;
    }
    glfwSetInputMode(window, GLFW_CURSOR, cursor);
    
    if(state.fixedWindowRatio)
	glfwSetWindowAspectRatio(window, winWidth, winHeight);

    render->LoadRender(window, state.conf);
}

Manager::~Manager() {
    delete render;
    glfwTerminate();
}

void Manager::update() {
    timer.Update();
    input.update();
    glfwPollEvents();
}

void Manager::setFullscreen(bool fullscreen) {
    const GLFWvidmode *mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    if(fullscreen) {
	glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width,
			     mode->height, mode->refreshRate);
    } else {
	glfwSetWindowMonitor(window, NULL, 0, 0, winWidth,
			     winHeight, mode->refreshRate);
    }
}

void Manager::toggleFullscreen() {
    setFullscreen(glfwGetWindowMonitor(window) == NULL);
}

glm::vec2 Manager::screenToRenderSpace(glm::vec2 pos) {
    glm::vec2 targetRes = render->getTargetResolution();
    if(targetRes.x != 0.0 && targetRes.y != 0.0)
      return glm::vec2(pos.x * (targetRes.x / (float)winWidth),
		       pos.y * (targetRes.y / (float)winHeight));
    return pos;
}

glm::vec2 Manager::mousePos() {
    return screenToRenderSpace(
	    glm::vec2(input.m.x(), input.m.y()));
}


//GLFW CALLBACKS

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  Manager *app = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  app->winWidth = width;
  app->winHeight = height;
  if(width != 0 && height != 0)
      app->render->FramebufferResize();
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
  Manager *app = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  app->input.m.mousePosCallback(xpos, ypos);
}
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  Manager *app = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  app->input.m.mouseScrollCallback(xoffset, yoffset);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                       int mode) {
  Manager *app = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  app->input.kb.handleKey(key, scancode, action);
}

void mouseBtnCallback(GLFWwindow *window, int button, int action,
                                int mods) {
  Manager *app = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  app->input.m.mouseButtonCallback(button, action, mods);
}

void errorCallback(int error, const char *description) {
  throw std::runtime_error(description);
}
