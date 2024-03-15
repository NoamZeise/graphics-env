#include <graphics/manager.h>
#include <game/camera.h>
#include <graphics/glm_helper.h>
#include <glm/gtc/matrix_inverse.hpp>

// This example shows minimum functionality of the graphics environment
// With none of the optional libs
//
// The program draws a textured quad
// Press Esc to exit.

#include "helper.h"

int main(int argc, char** argv) {
    ManagerState state;
    state.windowTitle = "minimum example";
    state.defaultRenderer = parseArgs(argc, argv, &state.windowTitle);

    Manager manager(state);
    
    ResourcePool* pool = manager.render->pool();
    Resource::Texture tex = pool->tex()->load("textures/tile.png");

    manager.render->LoadResourcesToGPU(pool);
    manager.render->UseLoadedResources();

    while(!glfwWindowShouldClose(manager.window)) {
	manager.update();
	
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);
	
	if(manager.winActive()) {
            manager.render->DrawQuad(
		    tex,
		    glmhelper::calcMatFromRect(
			    glm::vec4(10.0f, 10.0f, 200.0f, 200.0f), 0.0f, 0.5f),
		    glm::vec4(1.0f),
		    glmhelper::getTextureOffset(
			    tex.dim, glm::vec4(0.0f, 0.0f, tex.dim.x * 5, tex.dim.y * 5)));
	    manager.render->EndDraw();
	}
    }
}
