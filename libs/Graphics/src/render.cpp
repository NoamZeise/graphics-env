#include "render.h"
#include "../../OpenGLEnvironment/src/render.h"
#include "../../VulkanEnvironment/src/render.h"
#include <iostream>

#define _RENDER_NO_FN(vk, gl) switch(renderer) { \
   case RenderFramework::VULKAN: vk; break;    \
        case RenderFramework::OPENGL: gl; break;    \
    }

#define _RENDER_FN(...) switch(renderer) { \
        case RenderFramework::VULKAN: return vkRender->__VA_ARGS__; break;    \
        case RenderFramework::OPENGL: \
        default: return glRender->__VA_ARGS__; break;		\
    }

Render::Render(RenderFramework preferredRenderer) {
    switch (preferredRenderer) {
        case RenderFramework::VULKAN:
            if(vkenv::Render::LoadVulkan()) {
                renderer = RenderFramework::VULKAN;
                break;
            }
            std::cout << "Failed to load Vulkan, trying OpenGL\n";

        case RenderFramework::OPENGL:
            if(glenv::GLRender::LoadOpenGL()) {
                renderer = RenderFramework::OPENGL;
                break;
            }
            else {
                std::cout <<"Failed to load OpenGL\n";
                noApiLoaded = true;
            }
            break;
    }
}

Render::~Render() { _RENDER_NO_FN(delete vkRender, delete glRender); }

void Render::LoadRender(GLFWwindow *window){
  int width, height;
  glfwGetWindowSize(window, &width, &height);
  _RENDER_NO_FN(vkRender = new vkenv::Render(window, glm::vec2(width, height)),
		glRender = new glenv::GLRender(window, glm::vec2(width, height)))
    }

    void Render::LoadRender(GLFWwindow *window, glm::vec2 target){
        _RENDER_NO_FN(vkRender = new vkenv::Render(window, target),
                      glRender = new glenv::GLRender(window, target))}


    Resource::Texture Render::LoadTexture(std::string filepath){
        _RENDER_FN(LoadTexture(filepath))}

    Resource::Font Render::LoadFont(std::string filepath){
        _RENDER_FN(LoadFont(filepath))}

    Resource::Model Render::Load3DModel(std::string filepath){
        _RENDER_FN(Load3DModel(filepath))}

    Resource::Model Render::Load3DModel(ModelInfo::Model& model){
        _RENDER_FN(Load3DModel(model))}

Resource::Model Render::LoadAnimatedModel(std::string filepath, std::vector<Resource::ModelAnimation> *pGetAnimations){
        _RENDER_FN(LoadAnimatedModel(filepath, pGetAnimations))
}

void Render::LoadResourcesToGPU() { _RENDER_FN(LoadResourcesToGPU()) }
void Render::UseLoadedResources(){_RENDER_FN(UseLoadedResources())}


  void Render::Begin3DDraw() { _RENDER_FN(Begin3DDraw()) }
  void Render::BeginAnim3DDraw() { _RENDER_FN(BeginAnim3DDraw()) }
  void Render::Begin2DDraw() { _RENDER_FN(Begin2DDraw()) }
  void Render::DrawModel(Resource::Model model, glm::mat4 modelMatrix,
                         glm::mat4 normalMatrix){
      _RENDER_FN(DrawModel(model, modelMatrix, normalMatrix))}
  void Render::DrawModel(Resource::Model model, glm::mat4 modelMatrix,
		       glm::mat4 normalMatrix, glm::vec4 colour) {
      _RENDER_FN(DrawModel(model, modelMatrix, normalMatrix, colour))
  } 
  void Render::DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix,
                     glm::mat4 normalMatrix,
                     Resource::ModelAnimation *animation) {
    _RENDER_FN(DrawAnimModel(model, modelMatrix, normalMatrix, animation))
  }
  void Render::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix,
                glm::vec4 colour, glm::vec4 texOffset) {
    _RENDER_FN(DrawQuad(texture, modelMatrix, colour, texOffset))
  }
  void Render::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix,
                glm::vec4 colour) {
    _RENDER_FN(DrawQuad(texture, modelMatrix, colour))
  }
  void Render::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix) {
    _RENDER_FN(DrawQuad(texture, modelMatrix))
  }
  void Render::DrawString(Resource::Font font, std::string text, glm::vec2 position,
                  float size, float depth, glm::vec4 colour, float rotate) {
    _RENDER_FN(DrawString(font, text, position, size, depth, colour, rotate))
  }
void Render::DrawString(Resource::Font font, std::string text, glm::vec2 position,
                  float size, float depth, glm::vec4 colour) {
    _RENDER_FN(DrawString(font, text, position, size, depth, colour))
  }
float Render::MeasureString(Resource::Font font, std::string text, float size) {
    _RENDER_FN(MeasureString(font, text, size))
  }
void Render::EndDraw(std::atomic<bool> &submit) { _RENDER_FN(EndDraw(submit)) }

void Render::FramebufferResize() { _RENDER_FN(FramebufferResize()) }

void Render::set3DViewMatrixAndFov(glm::mat4 view, float fov, glm::vec4 camPos) {
    _RENDER_FN(set3DViewMatrixAndFov(view, fov, camPos))
  }
void Render::set2DViewMatrixAndScale(glm::mat4 view, float scale) {
    _RENDER_FN(set2DViewMatrixAndScale(view, scale))
  }
  void Render::setLightDirection(glm::vec4 lightDir){
      _RENDER_FN(setLightDirection(lightDir))}

glm::mat4 Render::get3DProj() {
    _RENDER_FN(get3DProj());
}
  
void Render::setForceTargetRes(bool force) { _RENDER_FN(setForceTargetRes(force)) }
bool Render::isTargetResForced() { _RENDER_FN(isTargetResForced()) }
void Render::setTargetResolution(glm::vec2 resolution){
    _RENDER_FN(setTargetResolution(resolution))}

glm::vec2 Render::getTargetResolution() {
    _RENDER_FN(getTargetResolution())
}
void Render::setVsync(bool vsync){_RENDER_FN(setVsync(vsync))}

bool Render::getVsync(){_RENDER_FN(getVsync())}
