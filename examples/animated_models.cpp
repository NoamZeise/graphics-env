#include <graphics/manager.h>
#include <game/camera.h>
#include <graphics/logger.h>
#include <glm/gtc/matrix_inverse.hpp>
#include "helper.h"

struct AnimatedModel {
    AnimatedModel(std::string modelpath, ModelLoader* load, float scale, glm::vec3 translate, std::string texPath) {
	const glm::mat4 basemat = glm::translate(glm::rotate(
							 glm::rotate(glm::mat4(1.0f),
								     glm::radians(270.0f),
								     glm::vec3(-1.0f, 0.0f, 0.0f)),
							 1.5f,
							 glm::vec3(0, 1, 0)),
						 glm::vec3(0.0f, -8.0f, -10.0f));
	this->modelMat = glm::translate(glm::scale(basemat, glm::vec3(scale)), translate);
	this->model = load->load(Resource::ModelType::m3D_Anim, modelpath, texPath, &animations);
	nextAnim();
    }
    void update(Timer &timer) {
	if(animations.size() > 0)
	    active.Update(timer.dt());
    }
    void nextAnim() {
	if(animations.size() == 0)
	    return;
	current++;
	if(current >= animations.size())
	    current = 0;
	active = animations[current];
    }
    void draw(Render* render) {
	if(animations.size() > 0)
	    render->DrawAnimModel(model, modelMat, glm::inverseTranspose(modelMat), &active);
    }
    Resource::Model model;
    glm::mat4 modelMat;
    std::vector<Resource::ModelAnimation> animations;
    int current = -1;
    Resource::ModelAnimation active;
};

int main(int argc, char** argv) {
    ManagerState state;
    state.cursor = cursorState::disabled;
    state.render.multisampling = true;
    state.render.sample_shading = true;
    state.render.mip_mapping = false;
    state.windowTitle = "Animated Models";
    state.defaultRenderer = parseArgs(argc, argv, &state.windowTitle);
    Manager manager(state);
    ResourcePool* pool = manager.render->pool();
    
    AnimatedModel wolf("models/wolf.fbx", pool->model(), 0.2f,
		       glm::vec3(-70.0f, 4.0f, 41.0f), "textures/");
    AnimatedModel robot("models/robot.gltf", pool->model(), 0.1f,
			glm::vec3(140.0f, -5.0f, 20.0f), "textures/robot/");
    Resource::Font font = pool->font()->load("textures/Roboto-Black.ttf");

    manager.render->LoadResourcesToGPU(pool);
    manager.render->UseLoadedResources();

    camera::FirstPerson cam;
    cam.setPos(glm::vec3(30.0f, 0, 0));

    BPLighting lighting;
    
    while(!glfwWindowShouldClose(manager.window)) {
	manager.update();

		
	if(manager.input.kb.hold(GLFW_KEY_UP)) {
	    lighting.direction.x += manager.timer.dt() * 0.001f;
	}
	if(manager.input.kb.hold(GLFW_KEY_DOWN)) {
	    lighting.direction.x -= manager.timer.dt() * 0.001f;
	}
	if(manager.input.kb.hold(GLFW_KEY_LEFT)) {
	    lighting.direction.y += manager.timer.dt() * 0.001f;
	}
	if(manager.input.kb.hold(GLFW_KEY_RIGHT)) {
	    lighting.direction.y -= manager.timer.dt() * 0.001f;
	}
	lighting.direction = glm::normalize(lighting.direction);
	manager.render->setLightingProps(lighting);
	
	
	if(manager.input.kb.press(GLFW_KEY_N)) {
	    wolf.nextAnim();
	    robot.nextAnim();
	}
	wolf.update(manager.timer);
	robot.update(manager.timer);
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);
	cam.flycamUpdate(manager.input, manager.timer);
	manager.render->set3DViewMat(cam.getView(), cam.getPos());
	
	if(manager.winActive()) {
	    wolf.draw(manager.render);
	    robot.draw(manager.render);

	    manager.render->DrawString(font, "Press N to cycle animations",
				       glm::vec2(20.0f, 30.0f),
				       20.0f, 1.0f, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));
	    manager.render->DrawString(font, "Wolf Anim: " + wolf.active.getName(),
				       glm::vec2(20.0f, 55.0f),
				       20.0f, 1.0f, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));
	    manager.render->DrawString(font, "Robot Anim: " + robot.active.getName(),
				       glm::vec2(20.0f, 75.0f),
				       20.0f, 1.0f, glm::vec4(1.0f, 0.5f, 1.0f, 1.0f));
	    manager.render->EndDraw();
	}
    }
}
