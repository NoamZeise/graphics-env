#include <manager.h>
#include <game/camera.h>
#include <graphics/glm_helper.h>
#include <glm/gtc/matrix_inverse.hpp> //for inverseTranspose
#include <cstring>

// This example shows the resource pool functionality

struct ModelDraw {
    Resource::Model model;
    glm::mat4 drawMat;
    glm::mat4 normalMat;
    Resource::ModelAnimation anim;
    bool animated = false;
    ModelDraw(){}
    ModelDraw(Resource::Model model, glm::mat4 mat);
    ModelDraw(Resource::Model model, glm::mat4 mat, Resource::ModelAnimation anim);
    void Update(float time);
    void Draw(Render *render);
    void setMat(glm::mat4 mat);
};

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
    ResourcePool* defaultPool = manager.render->pool();
    Resource::Font font = defaultPool->font()->load("textures/Roboto-Black.ttf");

    //create a new pool
    ResourcePool* pool1 = manager.render->CreateResourcePool();

    glm::mat4 base = glm::rotate(glm::mat4(1.0f), glm::radians(270.0f),
				 glm::vec3(-1.0f, 0.0f, 0.0f));
    
    //use this pool to load a model
    ModelDraw monkey = ModelDraw(
	    pool1->model()->load("models/monkey.obj"),
	    glm::translate(base, glm::vec3(0.0f, -8.0f, -10.0f)));
    ModelDraw sphere = ModelDraw(
	    pool1->model()->load("models/sphere.obj"), glm::mat4(1.0f));

    //create another pool
    ResourcePool* pool2 = manager.render->CreateResourcePool();

    // use this pool 2 to load a wolf with animations
    std::vector<Resource::ModelAnimation> wolfAnims;
    Resource::Model wolfModel = pool1->model()->load(
	    Resource::ModelType::m3D_Anim, "models/wolf.fbx", &wolfAnims);
    ModelDraw wolf = ModelDraw(
	    wolfModel,
	    glm::translate(glm::scale(base, glm::vec3(0.1f)),
			   glm::vec3(-25.0f, -50.0f, -80.0f)),
	    wolfAnims[0]);
    
    Resource::Texture tex = pool2->tex()->load("textures/tile.png");
    
    // load staged resources into gpu so we can use them for drawing
    manager.render->LoadResourcesToGPU(defaultPool);
    manager.render->LoadResourcesToGPU(pool1);
    manager.render->LoadResourcesToGPU(pool2);
    manager.render->UseLoadedResources();

    camera::ThirdPerson cam;
    cam.setForward(glm::vec3(-1, 0, 0));
    glm::vec3 spherePos(0, 1, 0);
    float radius = 5.0f;
    
    while(!glfwWindowShouldClose(manager.window)) {
	manager.update();
	
	wolf.Update(manager.timer.dt());
	if(manager.input.kb.press(GLFW_KEY_ESCAPE))
	    glfwSetWindowShouldClose(manager.window, GLFW_TRUE);

	float speed = 0.01f * manager.timer.dt();
	if(manager.input.kb.hold(GLFW_KEY_W))
	    spherePos += cam.getTargetForward() * speed;
	if(manager.input.kb.hold(GLFW_KEY_S))
	    spherePos -= cam.getTargetForward() * speed;
	if(manager.input.kb.hold(GLFW_KEY_A))
	    spherePos += cam.getTargetLeft() * speed;
	if(manager.input.kb.hold(GLFW_KEY_D))
	    spherePos -= cam.getTargetLeft() * speed;
	if(manager.input.kb.hold(GLFW_KEY_LEFT_SHIFT))
	    spherePos -= cam.worldUp * speed;
	if(manager.input.kb.hold(GLFW_KEY_SPACE))
	    spherePos += cam.worldUp * speed;

	sphere.setMat(glm::translate(glm::mat4(1.0f), spherePos));
	radius -= manager.input.m.scroll() * manager.timer.dt() * 0.01f;
	cam.setTarget(spherePos, radius);	
	cam.control(
		-glm::vec2(manager.input.m.dx(),manager.input.m.dy())
		* (float)manager.timer.dt() * 0.0001f);

	
	manager.render->set3DViewMat(cam.getView(), cam.getPos());

	if(manager.winActive()) {
	    wolf.Draw(manager.render);
	    monkey.Draw(manager.render);
	    sphere.Draw(manager.render);
            manager.render->DrawQuad(tex,
				     glmhelper::calcMatFromRect(
					     glm::vec4(10.0f, 10.0f, 200.0f, 200.0f),
					     0.0f, 1.0f),
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


ModelDraw::ModelDraw(Resource::Model model, glm::mat4 mat) {
    this->model = model;
    this->normalMat = glm::inverseTranspose(mat);
    setMat(mat);
}

ModelDraw::ModelDraw(Resource::Model model, glm::mat4 mat, Resource::ModelAnimation anim) {
    this->model = model;
    this->anim = anim;
    this->animated = true;
    setMat(mat);
}
void ModelDraw::Draw(Render *render) {
    if(animated)
	render->DrawAnimModel(model, drawMat, normalMat, &anim);
    else
	render->DrawModel(model, drawMat, normalMat);
}

void ModelDraw::Update(float time) {
    if(animated)
	anim.Update(time);
}

void ModelDraw::setMat(glm::mat4 mat) {
    this->drawMat = mat;
    this->normalMat = glm::inverseTranspose(mat);
}
