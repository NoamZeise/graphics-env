#include <graphics/manager.h>
#include <game/camera.h>
#include <graphics/glm_helper.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <graphics/logger.h>

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
    state.defaultRenderer = parseArgs(argc, argv, &state.windowTitle);
    state.cursor = cursorState::disabled;
    Manager manager(state);

    // example shader use

    /*
      DescriptorSet* timeds = render.createDs(stage::vertex | stage::fragment); 

      // option 1, easier, raw pointer
      Binding *time = timeds->add(0, Binding::UniformBuffer(sizeof(timestruct)));
      
      time->data(&timeData); // upload data, automatically update to gpu during call;
      //or
      *(timestruct*)time->data() = dt;
      //with offsets for arrays
      time->data(&timeData, array_index);
      //swap dynamic binding
      timeds->dynamicIndex(2);

      // option 2, somehow type checking, harder

      time->data = dt; // so data is of type time
      time->update(); // manually update to gpu
      // requires lots of casting and annoying stuff
      timeds->add(Binding::UniformBuffer<timestruct>());

      // option 3: wrapper object with type - pass array size in generic

      BindingData<timestruct> time(timeds->add(0, Binding::UniformBuffer(sizeof(timestruct))));
      time.data = dt;
      time.update();



      // after creation

      pipeline.attach(2, timeds); //set index, set
      
     */

    BPLighting l;
    l.specular.w = 30.0f;
    l.direction = glm::normalize(glm::vec4(0, 0.2, -1, 0));    
    manager.render->setLightingProps(l);
    
    ResourcePool* pool = manager.render->pool();
    Resource::Texture tex = pool->tex()->load("textures/tile.png");
    Resource::Model monkey = pool->model()->load("models/bunny.obj");
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 normMat = glm::inverseTranspose(model);

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
	    manager.render->DrawModel(monkey, model, normMat);	    
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
