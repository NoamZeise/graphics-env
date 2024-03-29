#include <graphics/manager.h>
#include <graphics/glm_helper.h>
// for parseArgs - choose opengl or vulkan backend
#include "helper.h"

// This example shows minimum functionality of the graphics environment
// With none of the optional libs
//
// The program draws a textured quad
// Press Esc to exit.

int main(int argc, char** argv) {
    // setup renderer with desired state
    ManagerState state;
    state.windowTitle = "minimum example";
    state.defaultRenderer = parseArgs(argc, argv, &state.windowTitle);
    Manager manager(state);
    
    ResourcePool* pool = manager.render->pool();
    // load resources to cpu
    Resource::Texture tex = pool->tex()->load("textures/tile.png");
    // load resources to gpu
    manager.render->LoadResourcesToGPU(pool);
    manager.render->UseLoadedResources();

    while(!glfwWindowShouldClose(manager.window)) {
	// update timers, input, etc
	manager.update();

	// Press Esc to exit
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);
	
	if(manager.winActive()) {
	    // draw texture where mouse is
            manager.render->DrawQuad(
		    tex,
		    glmhelper::calcMatFromRect(
			    glm::vec4(manager.mousePos().x, manager.mousePos().y, 200.0f, 200.0f)
			    , 0.0f, 0.5f),
		    glm::vec4(1.0f),
		    glmhelper::getTextureOffset(
			    tex.dim, glm::vec4(0.0f, 0.0f, tex.dim.x * 5, tex.dim.y * 5)));
	    manager.render->EndDraw();
	}
    }
}
