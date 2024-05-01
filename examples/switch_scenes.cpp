#include <atomic>
#include <thread>

#include <graphics/manager.h>
#include <game/camera.h>
#include <iostream>

#define TIME_APP_DRAW_UPDATE
//#define MULTI_UPDATE_ON_SLOW_DRAW

class App {
public:
    App(RenderFramework defaultFramework);
    ~App();
    void run();
private:
    void loadAssets();
    void update();
    void controls();
    void postUpdate();
    void draw();

    void loadTestScene1(std::atomic<bool> &loaded);
    void drawTestScene1();
    void loadTestScene2(std::atomic<bool> &loaded);
    void drawTestScene2();

    enum class Scene {
	Test1,
	Test2,
    };
    Scene current = Scene::Test1;

    Manager* manager;
    camera::FirstPerson fpcam;

    glm::vec4 lightDir = glm::vec4(0.0f, -0.5f, -1.0f, 0.0f);
    float rotate = 0.0;

    std::thread submitDraw;
    std::atomic<bool> finishedDrawSubmit;
    std::thread assetLoadThread;
    std::atomic<bool> assetsLoaded;

    bool sceneChangeInProgress = false;

    Resource::Model testModel1;
    Resource::Model monkeyModel1;
    Resource::Model colouredCube1;
    Resource::Model wolf1;
    Resource::ModelAnimation wolfAnim1;
    Resource::Texture testTex1;
    Resource::Font testFont1;

    Resource::Model testModel2;
    Resource::Model monkeyModel2;
    Resource::Model colouredCube2;
    Resource::Texture testTex2;
    Resource::Font testFont2;

#ifdef TIME_APP_DRAW_UPDATE
    std::string monitored_update_stats = "";
    std::string  monitored_draw_stats = "";
#endif
};

#include <stdexcept>
#include <graphics/logger.h>
#include <fstream>

#include "helper.h"

int main(int argc, char** argv) {
  try
    {
      std::string t("");
      App app(parseArgs(argc, argv, &t));
      app.run();
    }
  catch (const std::exception& e)
    {
#ifndef NDEBUG
	LOG_ERROR(e.what());
#else
      std::ofstream crashFile("CrashInfo.txt");
      if (crashFile.is_open())
	{
	  crashFile.seekp(0);
	  crashFile << e.what();
	  crashFile.close();
	}
#endif
      return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <graphics/glm_helper.h>
#include <graphics/shader_structs.h>
#include <graphics/logger.h>

App::App(RenderFramework defaultFramework) {
    ManagerState state;
    state.defaultRenderer = defaultFramework;
    state.windowTitle = std::string("Test App ") +
	(defaultFramework == RenderFramework::Vulkan ? "vulkan" : "opengl");
    state.cursor = cursorState::disabled;
    manager = new Manager(state);
    
    loadAssets();
    
    fpcam = camera::FirstPerson(glm::vec3(3.0f, 0.0f, 3.0f));
    finishedDrawSubmit = true;
    manager->audio.Play("audio/test.wav", false, 1.0f);
}

App::~App() {
  if (submitDraw.joinable())
    submitDraw.join();
  delete manager;
}

void App::loadAssets() {
    if(current == Scene::Test1)
	loadTestScene1(assetsLoaded);
    if(current == Scene::Test2)
	loadTestScene2(assetsLoaded);
    manager->render->LoadResourcesToGPU(Resource::Pool(0));
    manager->render->UseLoadedResources();
}

void App::run() {
  while (!glfwWindowShouldClose(manager->window)) {
    update();
    if (manager->winActive())
      draw();
  }
}

void App::update() {
#ifdef TIME_APP_DRAW_UPDATE
  auto start = std::chrono::high_resolution_clock::now();
#endif
  manager->update();

  controls();

  if(sceneChangeInProgress && assetsLoaded) {
      if(assetLoadThread.joinable())
	  assetLoadThread.join();
      if(submitDraw.joinable()) 
	  submitDraw.join();
      LOG("loading done");
      manager->render->LoadResourcesToGPU(Resource::Pool(0));
      manager->render->UseLoadedResources();
      sceneChangeInProgress = false;
      current = current == Scene::Test1 ? Scene::Test2 : Scene::Test1;
  }

  if(current == Scene::Test1) {
      wolfAnim1.Update(manager->timer.dt());
  }

  rotate += manager->timer.dt() * 0.001f;
  
  fpcam.flycamUpdate(manager->input, manager->timer);

  postUpdate();
#ifdef TIME_APP_DRAW_UPDATE
    monitored_update_stats = "update: " + std::to_string(
	  std::chrono::duration_cast<std::chrono::microseconds>(
		  std::chrono::high_resolution_clock::now() - start).count() / 1000.0) + " ms";
#endif
}

void App::controls() {
    if (manager->input.kb.press(GLFW_KEY_F))
	manager->toggleFullscreen();
    if (manager->input.kb.press(GLFW_KEY_ESCAPE)) {
	glfwSetWindowShouldClose(manager->window, GLFW_TRUE);
    }
    const float speed = 0.001f;
    if (manager->input.kb.press(GLFW_KEY_INSERT)) {
	lightDir.x += speed * manager->timer.dt();
    }
    if (manager->input.kb.press(GLFW_KEY_HOME)) {
	lightDir.x -= speed * manager->timer.dt();
    }
    if (manager->input.kb.press(GLFW_KEY_DELETE)) {
	lightDir.z += speed * manager->timer.dt();
    }
    if (manager->input.kb.press(GLFW_KEY_END)) {
	lightDir.z -= speed * manager->timer.dt();
    }
    if (manager->input.kb.press(GLFW_KEY_PAGE_UP)) {
	lightDir.y += speed * manager->timer.dt();
    }
    if (manager->input.kb.press(GLFW_KEY_PAGE_DOWN)) {
	lightDir.y -= speed * manager->timer.dt();
    }
    if (manager->input.kb.press(GLFW_KEY_G)) {
	RenderConfig conf = manager->render->getRenderConf();
	conf.target_resolution[0] = 1000;
	conf.target_resolution[1] = 100;
	manager->render->setRenderConf(conf);
    }
    if (manager->input.kb.press(GLFW_KEY_H)) {
	RenderConfig conf = manager->render->getRenderConf();
	conf.target_resolution[0] = 0;
	conf.target_resolution[1] = 0;
	manager->render->setRenderConf(conf);
    }
    if (manager->input.kb.press(GLFW_KEY_V)) {
	RenderConfig renderConf = manager->render->getRenderConf();
	renderConf.vsync = !renderConf.vsync;
	manager->render->setRenderConf(renderConf);
    }
    if(!sceneChangeInProgress) {
	if(manager->input.kb.hold(GLFW_KEY_N)) {
	    if(current != Scene::Test1) {
		assetsLoaded = false;
		assetLoadThread = std::thread(&App::loadTestScene1, this, std::ref(assetsLoaded));
		sceneChangeInProgress = true;
	    }
	    else if(current != Scene::Test2) {
		assetsLoaded = false;
		assetLoadThread = std::thread(&App::loadTestScene2, this, std::ref(assetsLoaded));
		sceneChangeInProgress = true;
	    }
	}
    }
    BPLighting lighting;
    lighting.direction = lightDir;
    manager->render->setLightingProps(lighting);
}

void App::postUpdate() {
  manager->render->set3DViewMat(fpcam.getView(), fpcam.getPos());
}

void App::draw() {
#ifdef TIME_APP_DRAW_UPDATE
  auto start = std::chrono::high_resolution_clock::now();
#endif
#ifdef MULTI_UPDATE_ON_SLOW_DRAW
  if (!finishedDrawSubmit)
    return;
  finishedDrawSubmit = false;
#endif
  if (submitDraw.joinable())
    submitDraw.join();
  
  if(current==Scene::Test1)
      drawTestScene1();
  if(current==Scene::Test2)
      drawTestScene2();

#ifdef TIME_APP_DRAW_UPDATE
  Resource::Font font;
  if(current == Scene::Test1)
      font = testFont1;
  if(current == Scene::Test2)
      font = testFont2;
  manager->render->DrawString(font, monitored_update_stats,
		      glm::vec2(10.0, 20.0), 15, 5.0f, glm::vec4(1.0f));
  manager->render->DrawString(font, monitored_draw_stats,
		      glm::vec2(10.0, 40.0), 15, 5.0f, glm::vec4(1.0f));
#endif

  if(manager->backend() == RenderFramework::Vulkan)
      submitDraw = std::thread([=]{manager->render->EndDraw(finishedDrawSubmit);});
  else
      manager->render->EndDraw(finishedDrawSubmit);
  
#ifdef TIME_APP_DRAW_UPDATE
  monitored_draw_stats = "draw: " + std::to_string(
	 1.0 / std::chrono::duration_cast<std::chrono::microseconds>(
		  std::chrono::high_resolution_clock::now() - start).count() * 1000000.0) + " fps";
#endif
}

void App::loadTestScene1(std::atomic<bool> &loaded) {
  LOG("loading scene 1");
  ResourcePool* p = manager->render->pool();
  testModel1 = p->model()->load("models/testScene.fbx");
  monkeyModel1 = p->model()->load("models/monkey.obj");
  colouredCube1 =p->model()->load("models/ROOM.fbx");
  std::vector<Resource::ModelAnimation> animations;
  wolf1 = p->model()->load(Resource::ModelType::m3D_Anim, "models/wolf.fbx", &animations);
  if(animations.size() > 2)
      wolfAnim1 = animations[1];
  else
      throw std::runtime_error("wolf anim had unexpected number of animations");
  testTex1 = p->tex()->load("textures/error.png");
  testFont1 = p->font()->load("textures/Roboto-Black.ttf");
  loaded = true;
}

void App::drawTestScene1() {

  auto model = glm::translate(
      glm::scale(
		 glm::rotate(glm::rotate(glm::mat4(1.0f), rotate, glm::vec3(0, 0, 1)),
                      glm::radians(270.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
          glm::vec3(1.0f)),
      glm::vec3(0, 3, 0));

  manager->render->DrawModel(monkeyModel1, model, glm::inverseTranspose(model));

  model = glm::translate(model, glm::vec3(0, 3, 0));

  manager->render->DrawModel(monkeyModel1, model, glm::inverseTranspose(model),
			     glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

  model = glm::translate(model, glm::vec3(0, 3, 0));

  manager->render->DrawModel(monkeyModel1, model, glm::inverseTranspose(model),
			     glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

  model = glm::translate(model, glm::vec3(0, 3, 0));
  
  manager->render->DrawModel(monkeyModel1, model, glm::inverseTranspose(model),
			     glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));

  model = glm::translate(
      glm::scale(
		 glm::rotate(glm::rotate(glm::mat4(1.0f), rotate, glm::vec3(0, 0, 1)),
                      glm::radians(270.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
          glm::vec3(0.01f)),
      glm::vec3(0, 0, 0));

  manager->render->DrawModel(testModel1, model, glm::inverseTranspose(model));

  model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f),
                                                glm::vec3(0.0f, -30.0f, -15.0f)),
                                 glm::radians(270.0f),
                                 glm::vec3(-1.0f, 0.0f, 0.0f)),
                     glm::vec3(4.0f));
  manager->render->DrawModel(colouredCube1, model, glm::inverseTranspose(model));

  model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f),
                                                glm::vec3(0.0f, 10.0f, 6.0f)),
                                 glm::radians(270.0f),
                                 glm::vec3(-1.0f, 0.0f, 0.0f)),
                     glm::vec3(0.1f));
  
  manager->render->DrawAnimModel(wolf1, model, glm::inverseTranspose(model), &wolfAnim1);


  manager->render->DrawString(testFont1, "Scene 1 - N to switch", glm::vec2(10, 100), 30, 1.0f, glm::vec4(1), 0.0f);
  if(sceneChangeInProgress) {
    manager->render->DrawString(testFont1, "Loading", glm::vec2(200, 400), 40, 1.0f, glm::vec4(1), 0.0f);
   }

     manager->render->DrawQuad(
	     testTex1, glmhelper::calcMatFromRect(glm::vec4(400, 100, 100, 100), 0, 1),
      glm::vec4(1), glm::vec4(0, 0, 1, 1));

  manager->render->DrawQuad(testTex1,
                    glmhelper::calcMatFromRect(glm::vec4(0, 0, 400, 400), 0, 0.1),
                    glm::vec4(1, 0, 1, 0.3), glm::vec4(0, 0, 1, 1));
}

void App::loadTestScene2(std::atomic<bool> &loaded) {
    LOG("loading scene 2");
    ResourcePool *p = manager->render->pool();
    monkeyModel2 = p->model()->load("models/monkey.obj");
    colouredCube2 = p->model()->load("models/ROOM.fbx");
    testFont2 = p->font()->load("textures/Roboto-Black.ttf");
    loaded = true;
}

void App::drawTestScene2() {
  auto model = glm::translate(
      glm::scale(
		 glm::rotate(glm::rotate(glm::mat4(1.0f), rotate, glm::vec3(0, 0, 1)),
                      glm::radians(270.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
          glm::vec3(1.0f)),
      glm::vec3(0, 2, 0));

  manager->render->DrawModel(monkeyModel2, model, glm::inverseTranspose(model));
  model = glm::translate(
      glm::scale(
		 glm::rotate(glm::rotate(glm::mat4(1.0f), rotate * 0.5f, glm::vec3(0, 0, 1)),
                      glm::radians(270.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
          glm::vec3(1.0f)),
      glm::vec3(1, 2, 0));
    manager->render->DrawModel(monkeyModel2, model, glm::inverseTranspose(model));
    model = glm::translate(
      glm::scale(
		 glm::rotate(glm::rotate(glm::mat4(1.0f), rotate * 2.0f, glm::vec3(0, 0, 1)),
                      glm::radians(270.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
          glm::vec3(1.0f)),
      glm::vec3(2, 2, 0));
    manager->render->DrawModel(monkeyModel2, model, glm::inverseTranspose(model));

      model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f),
                                                glm::vec3(0.0f, -30.0f, -15.0f))

                                     ,
                                 glm::radians(270.0f),
                                 glm::vec3(-1.0f, 0.0f, 0.0f)),
                     glm::vec3(1.0f));

      manager->render->DrawModel(colouredCube2, model, glm::inverseTranspose(model));
      model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f),
                                                glm::vec3(0.0f, -30.0f, -15.0f))

                                     ,
                                 glm::radians(270.0f),
                                 glm::vec3(-1.0f, 0.0f, 0.0f)),
                     glm::vec3(1.0f));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  manager->render->DrawModel(colouredCube2, model, glm::inverseTranspose(model));
        model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f),
                                                glm::vec3(0.0f, -30.0f, -15.0f))

                                     ,
                                 glm::radians(270.0f),
                                 glm::vec3(-1.0f, 0.0f, 0.0f)),
                     glm::vec3(1.0f));
  manager->render->DrawModel(colouredCube2, model, glm::inverseTranspose(model));

  manager->render->DrawString(testFont2, "Scene 2 - N to switch", glm::vec2(10, 100), 30, 1.0f, glm::vec4(1), 0.0f);

  if(sceneChangeInProgress) {
      manager->render->DrawString(testFont2, "Loading", glm::vec2(200, 400), 40, 1.0f, glm::vec4(1), 0.0f);
  }    
}
