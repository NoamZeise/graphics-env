#include <manager.h>
#include <GameHelper/camera.h>
#include <graphics/glm_helper.h>
#include <glm/gtc/matrix_inverse.hpp> //for inverseTranspose

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
    Manager manager(RenderFramework::VULKAN, state);

    Resource::Model monkey = manager.render->Load3DModel("models/monkey.obj");
    glm::mat4 monkeyMat = glm::translate(glm::rotate(glm::mat4(1.0f), glm::radians(270.0f),
						     glm::vec3(-1.0f, 0.0f, 0.0f)),
					 glm::vec3(0.0f, -8.0f, -10.0f));
    std::vector<Resource::ModelAnimation> wolfAnims;
    Resource::Model wolf = manager.render->LoadAnimatedModel("models/wolf.fbx", &wolfAnims);
    Resource::ModelAnimation anim = wolfAnims[0];
    glm::mat4 wolfMat = glm::translate(glm::scale(monkeyMat, glm::vec3(0.1f)),
				       glm::vec3(-25.0f, -50.0f, -80.0f));
    Resource::Font font = manager.render->LoadFont("textures/Roboto-Black.ttf");
    Resource::Texture tex = manager.render->LoadTexture("textures/tile.png");
    manager.render->LoadResourcesToGPU();
    manager.render->UseLoadedResources();

    camera::FirstPerson cam;
    manager.audio.Play("audio/test.wav", false, 1.0f);
    
    while(!glfwWindowShouldClose(manager.window)) {
	manager.update();
	anim.Update(manager.timer.FrameElapsed());
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);
	cam.update(manager.input, manager.timer);
	glm::vec4 camPos = glm::vec4(cam.getPos().x, cam.getPos().y, cam.getPos().z, 0.0f);
	manager.render->set3DViewMatrixAndFov(cam.getViewMatrix(), cam.getZoom(), camPos);

	if(manager.winWidth != 0 && manager.winHeight != 0) {
	    manager.render->BeginAnim3DDraw();
	    manager.render->DrawAnimModel(wolf, wolfMat, glm::inverseTranspose(wolfMat), &anim);
	    manager.render->Begin3DDraw();
	    manager.render->DrawModel(monkey, monkeyMat, glm::inverseTranspose(monkeyMat));
	    manager.render->Begin2DDraw();
            manager.render->DrawQuad(tex,
				     glmhelper::calcMatFromRect(
					     glm::vec4(10.0f, 10.0f, 200.0f, 200.0f),
					     0.0f, 0.0f),
				     glm::vec4(1.0f),
				     glmhelper::getTextureOffset(
					     tex.dim, glm::vec4(0.0f, 0.0f,
								tex.dim.x * 5,
								tex.dim.y * 5)));
	    manager.render->DrawString(font, "Graphics Environment", glm::vec2(220.0f, 50.0f),
				       50.0f, 1.0f, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));
	    std::atomic<bool> drawSubmitted;
	    manager.render->EndDraw(drawSubmitted);
	}
    }
}
