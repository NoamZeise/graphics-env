#include <graphics/manager.h>
#include <game/camera.h>
#include <graphics/glm_helper.h>
#include <glm/gtc/matrix_inverse.hpp>

#include "helper.h"

int main(int argc, char** argv) {
    ManagerState state;
    state.windowTitle = "Gooch Shading";
    state.defaultRenderer = parseArgs(argc, argv, &state.windowTitle);

    // set frag shader
    
    // set vert shader
    
    Manager manager(state);
    
    ResourcePool* pool = manager.render->pool();
    Resource::Texture tex = pool->tex()->load("textures/tile.png");
    Resource::Model monkey = pool->model()->load("models/monkey.obj");
    glm::mat4 model(1.0f);
    glm::mat4 normMat = glm::inverseTranspose(model);

    manager.render->LoadResourcesToGPU(pool);
    manager.render->UseLoadedResources();

    camera::ThirdPerson cam;
    cam.setTarget(glm::vec3(0), 10.0f);
    while(!glfwWindowShouldClose(manager.window)) {
	manager.update();
	cam.control(
		0.01f*manager.timer.dt()
		*glm::vec2(manager.input.m.dx(), manager.input.m.dy()));
	manager.render->set3DViewMat(cam.getView(), cam.getPos());
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);
	
	if(manager.winActive()) {
	    manager.render->DrawModel(monkey, model, normMat);	    
	    manager.render->EndDraw();
	}
    }
}
