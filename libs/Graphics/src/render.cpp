#include "render.h"

#ifndef NO_OPENGL
#include "../../OpenGLEnvironment/src/render.h"
#else
namespace glenv {
    class Render{};
}
#endif
#ifndef NO_VULKAN
#include "../../VulkanEnvironment/src/render.h"
#else
    namespace vkenv {
	class Render{};
    }
#endif
#include <iostream>

#ifndef NO_VULKAN
#define _VK_NO_FN(vk) vk
#define _VK_FN(...) return vkRender->__VA_ARGS__
#else
#include <stdexcept>    
#define _VK_NO_FN(vk)
#define _VK_FN(...) throw std::runtime_error("NO_VULKAN defined, tried to use render fn")
#endif

#ifndef NO_OPENGL
#define _GL_NO_FN(gl) gl
#define _GL_FN(...) return glRender->__VA_ARGS__
#else
#include <stdexcept>    
#define _GL_NO_FN(gl)
#define _GL_FN(...) throw std::runtime_error("NO_OPENGL defined, tried to use render fn")
#endif


#define _RENDER_NO_FN(vk, gl) switch(renderer) {	\
    case RenderFramework::VULKAN: _VK_NO_FN(vk); break;	\
    case RenderFramework::OPENGL: _GL_NO_FN(gl); break;	\
    }

//use ogl as default to stop complaints for no return values 
#define _RENDER_FN(...) switch(renderer) { \
    case RenderFramework::VULKAN: _VK_FN(__VA_ARGS__); break;	\
    case RenderFramework::OPENGL:					\
    default: _GL_FN(__VA_ARGS__);}

Render::Render(RenderFramework preferredRenderer) {
    switch (preferredRenderer) {
        case RenderFramework::VULKAN:
	    #ifndef NO_VULKAN
            if(vkenv::Render::LoadVulkan()) {
                renderer = RenderFramework::VULKAN;
                break;
            }
	    std::cout << "Failed to load Vulkan, trying OpenGL\n";
	    #else
	    std::cout << "Failed to load vulkan, NO_VULKAN was defined!\n";
	    #endif

        case RenderFramework::OPENGL:
	    #ifndef NO_OPENGL
            if(glenv::GLRender::LoadOpenGL()) {
                renderer = RenderFramework::OPENGL;
                break;
            }
            else {
                std::cout <<"Failed to load OpenGL\n";
                noApiLoaded = true;
            }
	    #else
	    std::cout << "Failed to load OpenGL, NO_OPENGL was defined!\n";	    
	    #endif
            break;
    }
}

Render::~Render() { _RENDER_NO_FN(delete vkRender, delete glRender); }

void Render::LoadRender(GLFWwindow *window){
    RenderConfig renderConf;
    _RENDER_NO_FN(vkRender = new vkenv::Render(window, renderConf),
		  glRender = new glenv::GLRender(window, renderConf));
}

    void Render::LoadRender(GLFWwindow *window, RenderConfig renderConf){
        _RENDER_NO_FN(vkRender = new vkenv::Render(window, renderConf),
                      glRender = new glenv::GLRender(window, renderConf))}


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
    _RENDER_FN(set3DViewMatrixAndFov(view, fov, camPos))}
void Render::set2DViewMatrixAndScale(glm::mat4 view, float scale) {_RENDER_FN(set2DViewMatrixAndScale(view, scale))}
void Render::setLightDirection(glm::vec4 lightDir) {_RENDER_FN(setLightDirection(lightDir))}
void Render::setTargetResolution(glm::vec2 resolution){_RENDER_FN(setTargetResolution(resolution))}

glm::vec2 Render::getTargetResolution(){_RENDER_FN(getTargetResolution())}

void Render::setRenderConf(RenderConfig renderConf) {_RENDER_FN(setRenderConf(renderConf))}

RenderConfig Render::getRenderConf() {_RENDER_FN(getRenderConf())}
