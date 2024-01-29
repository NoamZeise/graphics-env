#include <manager.h>
#include <game/camera.h>
#include <graphics/glm_helper.h>
#include <glm/gtc/matrix_inverse.hpp>

// This example shows the main functionality of the graphics environment
// With a minimum amount of code around it.
//
// The program draws a 3D model, and animated 3D model,
// a texture that tiles, and some text.
// It also plays audio and uses the 3D first person camera to
// move through the scene. (so you can use a keyboard/mouse or a controller)
// Press Esc to exit.

int main() {
    ManagerState state;
    state.cursor = cursorState::disabled;
    state.windowTitle = "minimum example";

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
            manager.render->DrawQuad(tex,
				     glmhelper::calcMatFromRect(
					     glm::vec4(10.0f, 10.0f, 200.0f, 200.0f),
					     0.0f, 0.5f),
				     glm::vec4(1.0f),
				     glmhelper::getTextureOffset(
					     tex.dim,
					     glm::vec4(0.0f, 0.0f, tex.dim.x * 5, tex.dim.y * 5)));
	    manager.render->EndDraw();
	}
    }
}
