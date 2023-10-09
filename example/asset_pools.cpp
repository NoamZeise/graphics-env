#include <manager.h>
#include <GameHelper/camera.h>
#include <graphics/glm_helper.h>
#include <glm/gtc/matrix_inverse.hpp> //for inverseTranspose
#include <cstring>

// This example shows the resource pool functionality

int main(int argc, char** argv) {
    RenderFramework framework = RenderFramework::Vulkan;
    for(int i = 0; i < argc; i++)
    	if(strcmp(argv[i], "opengl") == 0)
    	    framework = RenderFramework::OpenGL;
    ManagerState state;
    state.cursor = cursorState::disabled;
    state.windowTitle = std::string("Asset Pools Examples: ") +
	(framework == RenderFramework::Vulkan ? "vulkan" : "opengl");
    state.conf.multisampling = true;
    state.conf.sample_shading = true;
    state.conf.vsync = true;
    state.defaultRenderer = framework;
    Manager manager(state);

    //no pool, loads to default pool
    Resource::Font font = manager.render->LoadFont("textures/Roboto-Black.ttf");

    //create a new pool
    Resource::ResourcePool pool1 = manager.render->CreateResourcePool();

    Resource::Model monkey = manager.render->LoadModel(pool1, Resource::ModelType::m3D,
						       "models/monkey.obj", nullptr);
    glm::mat4 monkeyMat = glm::translate(glm::rotate(glm::mat4(1.0f), glm::radians(270.0f),
						     glm::vec3(-1.0f, 0.0f, 0.0f)),
					 glm::vec3(0.0f, -8.0f, -10.0f));
    
    Resource::ResourcePool pool2 = manager.render->CreateResourcePool();
    std::vector<Resource::ModelAnimation> wolfAnims;
    Resource::Model wolf = manager.render->LoadModel(pool1, Resource::ModelType::m3D_Anim,
						     "models/wolf.fbx", &wolfAnims);
    Resource::ModelAnimation anim = wolfAnims[0];
    glm::mat4 wolfMat = glm::translate(glm::scale(monkeyMat, glm::vec3(0.1f)),
				       glm::vec3(-25.0f, -50.0f, -80.0f));
    
    Resource::Texture tex = manager.render->LoadTexture(pool2, "textures/tile.png");
    manager.render->LoadResourcesToGPU();
    manager.render->LoadResourcesToGPU(pool1);
    manager.render->LoadResourcesToGPU(pool2);
    manager.render->UseLoadedResources();

    camera::FirstPerson cam;
    
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
