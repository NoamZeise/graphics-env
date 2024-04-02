#include <graphics/manager.h>

#include <stdexcept>
#include <iostream>
#include <graphics/logger.h>

#ifndef NO_OPENGL
#include "../libs/opengl/src/render.h"
#endif
#ifndef NO_VULKAN
#include "../libs/vulkan/src/render.h"
#endif

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void mouseCallback(GLFWwindow *window, double xpos, double ypos);
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset);
void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mode);
void mouseBtnCallback(GLFWwindow *window, int button, int action, int mods);
void errorCallback(int error, const char *description);

RenderFramework chooseRenderFramework(RenderFramework preferred);
void setShaders(shader::PipelineSetup* p, shader::pipeline pipeline,
		std::string default_vertex,
		std::string default_fragment);

Manager::Manager(ManagerState state) {
    winWidth = state.windowWidth;
    winHeight = state.windowHeight;

    glfwSetErrorCallback(errorCallback);
    if(!glfwInit())
	throw std::runtime_error("Failed to initialize GLFW!");
    
    framework = chooseRenderFramework(state.defaultRenderer);

    if(state.hideWindowOnCreate)
	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    
    window = glfwCreateWindow(winWidth, winHeight, state.windowTitle.c_str(),
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

    shader::PipelineSetup pl = state.getPipelineSetup(framework);
    switch(framework) {
    case RenderFramework::Vulkan:
#ifndef NO_VULKAN
	setShaders(&pl, shader::pipeline::_3D,
		  "shaders/vulkan/3D-lighting.vert.spv",
		  "shaders/vulkan/blinnphong.frag.spv");
	setShaders(&pl, shader::pipeline::anim3D,
		  "shaders/vulkan/3D-lighting-anim.vert.spv",
		  "shaders/vulkan/blinnphong.frag.spv");
	setShaders(&pl, shader::pipeline::_2D,
		  "shaders/vulkan/flat.vert.spv",
		  "shaders/vulkan/flat.frag.spv");
	setShaders(&pl, shader::pipeline::final,
		  "shaders/vulkan/final.vert.spv",
		  "shaders/vulkan/final.frag.spv");
	render = static_cast<Render*>(new vkenv::RenderVk(window, state.render, pl));
	break;
#endif
    case RenderFramework::OpenGL:
#ifndef NO_OPENGL
	setShaders(&pl, shader::pipeline::_3D,
		  "shaders/opengl/3D-lighting.vert",
		  "shaders/opengl/blinnphong.frag");
	setShaders(&pl, shader::pipeline::anim3D,
		  "shaders/opengl/3D-lighting-anim.vert",
		  "shaders/opengl/blinnphong.frag");
	setShaders(&pl, shader::pipeline::_2D,
		  "shaders/opengl/flat.vert",
		  "shaders/opengl/flat.frag");
	setShaders(&pl, shader::pipeline::final,
		  "shaders/opengl/final.vert",
		  "shaders/opengl/final.frag");
	render = static_cast<Render*>(new glenv::RenderGl(window, state.render, pl));
	break;
#endif
    default:
	throw std::runtime_error("Graphics Env manager: No renderer could be created!");
    }
    for(int i = 0; i < shader::PIPELINE_COUNT; i++) {
	for(int j = 0; j < shader::SHADER_STAGE_COUNT; j++) {
	    LOG("shader path: " << pl.getPath((shader::pipeline)i, (shader::stage)j));
	}
    }
}

Manager::~Manager() {
    delete render;
    glfwTerminate();
}

void Manager::update() {
    timer.update();
    input.update();
    glfwPollEvents();

    glm::mat4 proj3d =
	glm::perspective(
		fov,
		render->offscreenSize().x / render->offscreenSize().y,
		render->getRenderConf().depth_range_3D[0],
		render->getRenderConf().depth_range_3D[1]);
    render->set3DProjMat(proj3d);
    glm::mat4 proj2d =
	glm::ortho(0.0f,
		   render->offscreenSize().x * scale2d,
		   render->offscreenSize().y * scale2d,
		   0.0f,
		   render->getRenderConf().depth_range_2D[0],
		   render->getRenderConf().depth_range_2D[1]);
    render->set2DProjMat(proj2d);
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

void Manager::setWindowSize(int width, int height) {
    if(glfwGetWindowMonitor(window) == NULL) {	
	winWidth = width;
	winHeight = height;
	glfwSetWindowSize(window, winWidth, winHeight);
    }
}

void Manager::_windowSizeCallback(int width, int height){
    winWidth = width;
    winHeight = height;
    input.m.reset();
}

glm::vec2 Manager::screenToRenderSpace(glm::vec2 pos) {
    RenderConfig conf = render->getRenderConf();
    
    if(conf.target_resolution[0] != 0.0 && conf.target_resolution[1] != 0.0)
      return glm::vec2(pos.x * (conf.target_resolution[0] / (float)winWidth),
		       pos.y * (conf.target_resolution[1] / (float)winHeight));
    return pos;
}

glm::vec2 Manager::mousePos() {
    return screenToRenderSpace(
	    glm::vec2(input.m.x(), input.m.y()));
}

RenderFramework Manager::backend() {
    return framework;
}

glm::vec2 Manager::winSize() { return glm::vec2(winWidth, winHeight); }

bool Manager::winActive() { return winHeight != 0 && winWidth != 0; }

void ManagerState::setShader(RenderFramework framework,
			     shader::pipeline pipeline,
			     shader::stage stage,
			     std::string path) {
    this->pipeline[(int)framework].setPath(pipeline, stage, path);
}

void ManagerState::setShader(shader::pipeline pipeline,
			     shader::stage stage,
			     std::string vulkan_path,
			     std::string ogl_path) {
    setShader(RenderFramework::Vulkan, pipeline, stage, vulkan_path);
    setShader(RenderFramework::OpenGL, pipeline, stage, ogl_path);
}

shader::PipelineSetup ManagerState::getPipelineSetup(RenderFramework framework) {
    return this->pipeline[(int)framework];
}

// ---- HELPERS ----

RenderFramework chooseRenderFramework(RenderFramework preferred) {
    switch (preferred) {
    case RenderFramework::Vulkan:
#ifndef NO_VULKAN
	if(vkenv::RenderVk::LoadVulkan())
	    return RenderFramework::Vulkan;
	LOG_ERROR("Failed to load Vulkan, trying OpenGL\n");
#else
	LOG_ERROR("Failed to load vulkan, NO_VULKAN was defined!\n");
#endif	
    case RenderFramework::OpenGL:
#ifndef NO_OPENGL
	if(glenv::RenderGl::LoadOpenGL())
	    return RenderFramework::OpenGL;
	else
	    LOG_ERROR("Failed to load OpenGL\n");
#else
	LOG_ERROR("Failed to load OpenGL, NO_OPENGL was defined!\n");
#endif
    }
    throw std::runtime_error("Failed to load any graphics apis!");
}

void setShaders(shader::PipelineSetup* p, shader::pipeline pipeline,
		std::string default_vertex,
		std::string default_fragment) {
    if(p->getPath(pipeline, shader::stage::vert).size() == 0)
	p->setPath(pipeline, shader::stage::vert, default_vertex);
    if(p->getPath(pipeline, shader::stage::frag).size() == 0)
	p->setPath(pipeline, shader::stage::frag, default_fragment);
}

// ---- GLFW CALLBACKS ----

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  Manager *manager = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  manager->_windowSizeCallback(width, height);
  if(width != 0 && height != 0)
      manager->render->FramebufferResize();
}

void mouseCallback(GLFWwindow *window, double xpos, double ypos) {
  Manager *manager = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  manager->input.m.mousePosCallback(xpos, ypos);
}
void scrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  Manager *manager = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  manager->input.m.mouseScrollCallback(xoffset, yoffset);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                       int mode) {
  Manager *manager = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  manager->input.kb.handleKey(key, scancode, action);
}

void mouseBtnCallback(GLFWwindow *window, int button, int action,
                                int mods) {
  Manager *manager = reinterpret_cast<Manager *>(glfwGetWindowUserPointer(window));
  manager->input.m.mouseButtonCallback(button, action, mods);
}

void errorCallback(int error, const char *description) {
  throw std::runtime_error(description);
}
