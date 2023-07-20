#include "app.h"
#include "render.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <graphics/glm_helper.h>
#include <iostream>

App::App(RenderFramework defaultFramework) {
    ManagerState state;
    manager = new Manager(defaultFramework, state);

    loadAssets();

    fpcam = camera::FirstPerson(glm::vec3(3.0f, 0.0f, 2.0f));
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
    manager->render->LoadResourcesToGPU();
    manager->render->UseLoadedResources();
}

void App::run() {
  while (!glfwWindowShouldClose(manager->window)) {
    update();
    if (manager->winWidth != 0 && manager->winHeight != 0)
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
      std::cout << "loading done\n";
      manager->render->LoadResourcesToGPU();
      manager->render->UseLoadedResources();
      sceneChangeInProgress = false;
      current = current == Scene::Test1 ? Scene::Test2 : Scene::Test1;
  }

  if(current == Scene::Test1) {
      wolfAnim1.Update(manager->timer.FrameElapsed());
  }

  rotate += manager->timer.FrameElapsed() * 0.001f;
  
  fpcam.update(manager->input, manager->timer);

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
	lightDir.x += speed * manager->timer.FrameElapsed();
    }
    if (manager->input.kb.press(GLFW_KEY_HOME)) {
	lightDir.x -= speed * manager->timer.FrameElapsed();
    }
    if (manager->input.kb.press(GLFW_KEY_DELETE)) {
	lightDir.z += speed * manager->timer.FrameElapsed();
    }
    if (manager->input.kb.press(GLFW_KEY_END)) {
	lightDir.z -= speed * manager->timer.FrameElapsed();
    }
    if (manager->input.kb.press(GLFW_KEY_PAGE_UP)) {
	lightDir.y += speed * manager->timer.FrameElapsed();
    }
    if (manager->input.kb.press(GLFW_KEY_PAGE_DOWN)) {
	lightDir.y -= speed * manager->timer.FrameElapsed();
    }
    if (manager->input.kb.press(GLFW_KEY_G)) {
	manager->render->setTargetResolution(glm::vec2(1000, 100));
    }
    if (manager->input.kb.press(GLFW_KEY_H)) {
	manager->render->setTargetResolution(glm::vec2(0.0f, 0.0f));
    }
    if (manager->input.kb.press(GLFW_KEY_V)) {
	RenderConfig renderConf = manager->render->getRenderConf();
	renderConf.vsync = !renderConf.vsync;
	manager->render->setRenderConf(renderConf);
    }
    if(!sceneChangeInProgress) {
	if(manager->input.kb.hold(GLFW_KEY_1)) {
	    if(current != Scene::Test1) {
		assetsLoaded = false;
		pFrameworkSwitch(manager->render,
				 assetLoadThread = std::thread(&App::loadTestScene1, this, std::ref(assetsLoaded)),
				 loadTestScene1(assetsLoaded)
				 );
		sceneChangeInProgress = true;
	    }
	}
	if(manager->input.kb.hold(GLFW_KEY_2)) {
	    if(current != Scene::Test2) {
		assetsLoaded = false;
		pFrameworkSwitch(manager->render,
				 assetLoadThread = std::thread(&App::loadTestScene2, this, std::ref(assetsLoaded)),
				 loadTestScene2(assetsLoaded)
				 );
		sceneChangeInProgress = true;
	    }
	}
    }
    manager->render->setLightDirection(lightDir);
}

void App::postUpdate() {
  manager->render->set3DViewMatrixAndFov(fpcam.getViewMatrix(), fpcam.getZoom(),
                                 glm::vec4(fpcam.getPos(), 0.0));
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

  pFrameworkSwitch(manager->render,
		   submitDraw = std::thread(
			   &Render::EndDraw, manager->render, std::ref(finishedDrawSubmit)),
		   {
		       manager->render->EndDraw(finishedDrawSubmit);
		       finishedDrawSubmit = true;
		   }
  );

#ifdef TIME_APP_DRAW_UPDATE
  auto stop = std::chrono::high_resolution_clock::now();
  monitored_draw_stats = "draw: " + std::to_string(
	 1.0 / std::chrono::duration_cast<std::chrono::microseconds>(
		  std::chrono::high_resolution_clock::now() - start).count() * 1000000.0) + " fps";
#endif
}

void App::loadTestScene1(std::atomic<bool> &loaded) {
  testModel1 = manager->render->Load3DModel("models/testScene.fbx");
  monkeyModel1 = manager->render->Load3DModel("models/monkey.obj");
  colouredCube1 = manager->render->Load3DModel("models/ROOM.fbx");
  std::vector<Resource::ModelAnimation> animations;
  wolf1 = manager->render->LoadAnimatedModel("models/wolf.fbx", &animations);
  if(animations.size() > 2)
      wolfAnim1 = animations[1];
  else
      throw std::runtime_error("wolf anim had unexpected number of animations");
  testTex1 = manager->render->LoadTexture("textures/error.png");
  testFont1 = manager->render->LoadFont("textures/Roboto-Black.ttf");
  loaded = true;
}

void App::drawTestScene1() {

  manager->render->Begin3DDraw();

  auto model = glm::translate(
      glm::scale(
		 glm::rotate(glm::rotate(glm::mat4(1.0f), rotate, glm::vec3(0, 0, 1)),
                      glm::radians(270.0f), glm::vec3(-1.0f, 0.0f, 0.0f)),
          glm::vec3(1.0f)),
      glm::vec3(0, 3, 0));

  manager->render->DrawModel(monkeyModel1, model, glm::inverseTranspose(model));

  model = glm::translate(model,
			 glm::vec3(0, 3, 0));

  manager->render->DrawModel(monkeyModel1, model, glm::inverseTranspose(model),
			     glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

    model = glm::translate(model,
			 glm::vec3(0, 3, 0));

  manager->render->DrawModel(monkeyModel1, model, glm::inverseTranspose(model),
			     glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));

    model = glm::translate(model,
			 glm::vec3(0, 3, 0));

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

  manager->render->BeginAnim3DDraw();

  model = glm::scale(glm::rotate(glm::translate(glm::mat4(1.0f),
                                                glm::vec3(0.0f, 10.0f, 6.0f)),
                                 glm::radians(270.0f),
                                 glm::vec3(-1.0f, 0.0f, 0.0f)),
                     glm::vec3(0.1f));
  
  manager->render->DrawAnimModel(wolf1, model, glm::inverseTranspose(model), &wolfAnim1);

  manager->render->Begin2DDraw();


  manager->render->DrawString(testFont1, "Scene 1", glm::vec2(10, 100), 40, -1.0f, glm::vec4(1), 0.0f);
  if(sceneChangeInProgress) {
    manager->render->DrawString(testFont1, "Loading", glm::vec2(200, 400), 40, -1.0f, glm::vec4(1), 0.0f);
   }

     manager->render->DrawQuad(
	     testTex1, glmhelper::calcMatFromRect(glm::vec4(400, 100, 100, 100), 0, -1),
      glm::vec4(1), glm::vec4(0, 0, 1, 1));

  manager->render->DrawQuad(testTex1,
                    glmhelper::calcMatFromRect(glm::vec4(0, 0, 400, 400), 0, 0),
                    glm::vec4(1, 0, 1, 0.3), glm::vec4(0, 0, 1, 1));
}

void App::loadTestScene2(std::atomic<bool> &loaded) {
  monkeyModel2 = manager->render->Load3DModel("models/monkey.obj");
  colouredCube2 = manager->render->Load3DModel("models/ROOM.fbx");
  testFont2 = manager->render->LoadFont("textures/Roboto-Black.ttf");
  loaded = true;
}

void App::drawTestScene2() {
    manager->render->Begin3DDraw();
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
      

  manager->render->Begin2DDraw();

  manager->render->DrawString(testFont2, "Scene 2", glm::vec2(10, 100), 40, -0.4f, glm::vec4(1), 0.0f);

  if(sceneChangeInProgress) {
      manager->render->DrawString(testFont2, "Loading", glm::vec2(200, 400), 40, -0.4f, glm::vec4(1), 0.0f);
  }    
}
