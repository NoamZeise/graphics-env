#include <graphics/manager.h>
#include <game/camera.h>
#include <graphics/glm_helper.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <graphics/logger.h>

#include <graphics/default_vertex_types.h>

#include "helper.h"


glm::vec3 camControls(Input &input);

int main(int argc, char** argv) {
    ManagerState state;
    state.windowTitle = "Gooch Shading";
    state.render.forceFinalBuffer = true;
    state.render.clear_colour[0] = 0.25f;
    state.render.clear_colour[1] = 0.25f;
    state.render.clear_colour[2] = 0.25f;
    state.render.depth_range_3D[0] = 0.001f;
    state.render.depth_range_3D[1] = 100.0f;
    state.render.multisampling = true;
    state.setShader(shader::pipeline::_3D, shader::stage::frag,
		    "vk-shaders/gooch.frag.spv", "ogl-shaders/gooch.frag");
    parseArgs(argc, argv, &state);
    state.cursor = cursorState::disabled;
    Manager manager(state);    

    BPLighting l;
    l.specular.w = 30.0f;
    l.direction = glm::normalize(glm::vec4(0, 0.2, -1, 0));    
    manager.render->setLightingProps(l);
    
    ResourcePool* pool = manager.render->pool();
    Resource::Model bunny = pool->model()->load("models/bunny.obj");

    manager.render->LoadResourcesToGPU(pool);
    manager.render->UseLoadedResources();

    camera::ThirdPerson cam;
    float camradius = 1.3f;
    cam.setTarget(glm::vec3(0, 0, 0), camradius);
    cam.setForward(glm::vec3(1, 0, -0.4));
    int timeSinceInput = 1000;    
    while(!glfwWindowShouldClose(manager.window)) {
	manager.update();
	float dt = manager.timer.dt();	
	
	glm::vec3 input = camControls(manager.input);
	if(input.x == 0 && input.y == 0) {
	    timeSinceInput += dt;
	    if(timeSinceInput > 1000)
		input.x = dt * 0.1f;
	} else
	    timeSinceInput = 0;
	cam.control(0.0001f * dt * glm::vec2(input.x, input.y));
	camradius -= input.z*dt*0.001f;
	cam.setTarget(camradius);
	
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);
	if(manager.input.kb.press(GLFW_KEY_F))
	    manager.toggleFullscreen();
	
	manager.render->set3DViewMat(cam.getView(), cam.getPos());
	
	if(manager.winActive()) {
	    manager.render->DrawModel(bunny, glm::mat4(1.0f), glm::mat4(1.0f));	    
	    manager.render->EndDraw();
	}
    }
}

glm::vec3 camControls(Input &input) {
    glm::vec3 rawInput = glm::vec3(-input.m.dx(), input.m.dy(), 0) +
	5.0f*glm::vec3((int)input.kb.hold(GLFW_KEY_D) -
		       (int)input.kb.hold(GLFW_KEY_A),
		       (int)input.kb.hold(GLFW_KEY_S) -
		       (int)input.kb.hold(GLFW_KEY_W), 0);
    
    rawInput.z = input.m.scroll() +
	(int)input.kb.hold(GLFW_KEY_Q) -
	(int)input.kb.hold(GLFW_KEY_E);

    return rawInput;
}
