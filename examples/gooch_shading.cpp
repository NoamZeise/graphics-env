#include <graphics/manager.h>
#include <game/camera.h>
#include <graphics/glm_helper.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <graphics/logger.h>

#include "helper.h"

int main(int argc, char** argv) {
    ManagerState state;
    state.windowTitle = "Gooch Shading";
    state.defaultRenderer = parseArgs(argc, argv, &state.windowTitle);
    state.setShader(shader::pipeline::_3D, shader::stage::frag,
		    "vk-shaders/gooch.frag.spv", "ogl-shaders/gooch.frag");
    Manager manager(state);
    
    ResourcePool* pool = manager.render->pool();
    Resource::Texture tex = pool->tex()->load("textures/tile.png");
    Resource::Model monkey = pool->model()->load("models/monkey.obj");
    glm::mat4 model(1.0f);
    glm::mat4 normMat = glm::inverseTranspose(model);

    manager.render->LoadResourcesToGPU(pool);
    manager.render->UseLoadedResources();

    camera::ThirdPerson cam;
    cam.setTarget(glm::vec3(0), 4.0f);
    cam.setForward(glm::vec3(1, 0, 0.8));
    int timeSinceInput = 1000;
    
    while(!glfwWindowShouldClose(manager.window)) {

	manager.update();

        glm::vec2 userInput = 0.0001f * manager.timer.dt()
	    * glm::vec2(-manager.input.m.dx(), manager.input.m.dy());
	
	if(userInput == glm::vec2(0)) {
	    timeSinceInput += manager.timer.dt();
	    if(timeSinceInput > 1000)
		userInput.x = manager.timer.dt() * 0.0001f;
	} else
	    timeSinceInput = 0;

	cam.control(userInput);
	
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);
	   
	manager.render->set3DViewMat(cam.getView(), cam.getPos());

	if(manager.winActive()) {
	    manager.render->DrawModel(monkey, model, normMat);	    
	    manager.render->EndDraw();
	}
    }
}
