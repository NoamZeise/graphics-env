#include <manager.h>
#include <game/camera.h>
#include <graphics/logger.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <cstring>

struct AnimatedModel {
    AnimatedModel(std::string modelpath, ModelLoader* load, float scale, glm::vec3 translate) {
	const glm::mat4 basemat = glm::translate(
		glm::rotate(glm::mat4(1.0f), glm::radians(270.0f),
			    glm::vec3(-1.0f, 0.0f, 0.0f)),
		glm::vec3(0.0f, -8.0f, -10.0f));
	this->modelMat = glm::translate(glm::scale(basemat, glm::vec3(scale)), translate);
	this->model = load->load(Resource::ModelType::m3D_Anim, modelpath, &animations);
	if(animations.size() > 0)
	    active = animations[0];
	else
	    LOG_ERROR("no animations on animated model");
    }
    void update(Timer &timer) {
	if(animations.size() > 0)
	    active.Update(timer.dt());
    }
    void draw(Render* render) {
	if(animations.size() > 0)
	    render->DrawAnimModel(model, modelMat, glm::inverseTranspose(modelMat), &active);
    }
    Resource::Model model;
    glm::mat4 modelMat;
    std::vector<Resource::ModelAnimation> animations;
    Resource::ModelAnimation active;
};

int main(int argc, char** argv) {
    ManagerState state;
    state.cursor = cursorState::disabled;
    state.conf.multisampling = true;
    state.conf.sample_shading = true;
    RenderFramework framework = RenderFramework::Vulkan;
    for(int i = 0; i < argc; i++)
    	if(strcmp(argv[i], "opengl") == 0)
    	    framework = RenderFramework::OpenGL;
    state.defaultRenderer = framework;
    state.windowTitle = std::string("Animated Example: ") +
	(framework == RenderFramework::Vulkan ? "vulkan" : "opengl");
    Manager manager(state);
    ResourcePool* pool = manager.render->pool();
    
    AnimatedModel wolf("models/wolf.fbx", pool->model(), 0.1f,
		       glm::vec3(-25.0f, -50.0f, 100.0f));
    
    AnimatedModel robot("models/robot.gltf", pool->model(), 0.1f,
			glm::vec3(10.0f, 30.0f, 100.0f));
    
    Resource::Font font = pool->font()->load("textures/Roboto-Black.ttf");
    manager.render->LoadResourcesToGPU(pool);
    manager.render->UseLoadedResources();

    camera::FirstPerson cam;
    cam.setPos(glm::vec3(30.0f, 0, 0));
    
    while(!glfwWindowShouldClose(manager.window)) {
	manager.update();
	wolf.update(manager.timer);
	robot.update(manager.timer);
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);
	cam.flycamUpdate(manager.input, manager.timer);
	manager.render->set3DViewMat(cam.getView(), cam.getPos());
	
	if(manager.winActive()) {
	    wolf.draw(manager.render);
	    robot.draw(manager.render);
	    manager.render->DrawString(font, "Animated Example", glm::vec2(20.0f, 50.0f),
				       30.0f, 1.0f, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));
	    std::atomic<bool> drawSubmitted;
	    manager.render->EndDraw(drawSubmitted);
	}
    }
}
