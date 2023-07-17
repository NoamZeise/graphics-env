#ifndef APP_H
#define APP_H

#include <atomic>
#include <thread>

#define GLFW_INCLUDE_NONE
#include <render.h>

#include <GLFW/glfw3.h>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <manager.h>
#include <GameHelper/camera.h>

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

#endif
