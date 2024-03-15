#ifndef OGL_RENDER_H
#define OGL_RENDER_H

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <string>
#include <atomic>
#include <vector>
#include <iostream>

#include <graphics/render.h>
#include <graphics/shader_structs.h>

#include "framebuffer.h"

class GLVertexData;
class GLPoolManager;

namespace glenv {
  class GLShader;
  const int MAX_DRAWS = 10000;

  //match in shaders
  const int MAX_3D_ANIM_BATCH = 1;

  class RenderGl : public Render {
  public:
      static bool LoadOpenGL();
      RenderGl(GLFWwindow* window, RenderConfig renderConf);
      ~RenderGl();

      ResourcePool* CreateResourcePool() override;
      void DestroyResourcePool(Resource::Pool pool) override;
      // does notging in OGL version
      void setResourcePoolInUse(Resource::Pool pool, bool usePool) override {}
      ResourcePool* pool(Resource::Pool pool) override;
      
      void LoadResourcesToGPU(Resource::Pool pool) override;
      // does nothing in OGL version
      void UseLoadedResources() override {}

      void DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMat) override;
      void DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix,
			 glm::mat4 normalMatrix,
			 Resource::ModelAnimation *animation) override;
      void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix,
		    glm::vec4 colour, glm::vec4 texOffset) override;
      void DrawString(Resource::Font font, std::string text, glm::vec2 position,
		      float size, float depth, glm::vec4 colour, float rotate) override;
      void EndDraw(std::atomic<bool> &submit) override;

      void FramebufferResize() override;

      void set3DViewMat(glm::mat4 view, glm::vec4 camPos) override;
      void set2DViewMat(glm::mat4 view) override;
      void set3DProjMat(glm::mat4 proj) override;
      void set2DProjMat(glm::mat4 proj) override;
      void setLightingProps(BPLighting lighting) override;
      void setRenderConf(RenderConfig renderConf) override;
      RenderConfig getRenderConf() override;
      glm::vec2 offscreenSize() override;

  private:
      Resource::Model loadModel(Resource::ModelType type, ModelInfo::Model model,
				std::vector<Resource::ModelAnimation> *pGetAnimations);
      Resource::Model loadModel(Resource::ModelType type, std::string filepath,
				std::vector<Resource::ModelAnimation> *pGetAnimations);
      void draw2DBatch(int drawCount, Resource::Texture texture,
		       glm::vec4 currentColour);
      void draw3DBatch(int drawCount, Resource::Model model);
      void draw3DAnim(Resource::Model model);
      void setVPshader(GLShader *shader);
      void setLightingShader(GLShader *shader);

      bool _validPool(Resource::Pool pool);
      void _throwIfPoolInvaid(Resource::Pool pool);
      bool _poolInUse(Resource::Pool pool);

      BPLighting lighting;
      RenderConfig renderConf;
      RenderConfig prevRenderConf;

      GLFWwindow *window;
      glm::vec2 windowResolution;

      GLShader *shader3D;
      GLShader *shader3DAnim;
      GLShader *flatShader;
      GLShader *finalShader;

      bool useOffscreenFramebuffer = true;
      GlFramebuffer* offscreenFramebuffer = nullptr;
      GlFramebuffer* offscreenBlitFramebuffer = nullptr;
      int msaaSamples = 1;

      glm::mat4 finalTransform = glm::mat4(1.0f);
      
      glm::mat4 proj2D;
      glm::mat4 view2D;

      glm::mat4 proj3D;
      glm::mat4 view3D;

      bool inDraw = false;

      Resource::Pool defaultPool;
      GLPoolManager* pools;

      enum class DrawMode {
	  d2D,
	  d3D,
	  d3DAnim,
      };

      void setShaderForMode(DrawMode mode, unsigned int i);

      struct Draw2D {
	  Draw2D() {}
	  Draw2D(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset);
	  Resource::Texture tex;
	  glm::mat4 model;
	  glm::vec4 colour;
	  glm::vec4 texOffset;
      };
      struct Draw3D {
	  Draw3D() {}
	  Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix);
      
	  Resource::Model model;
	  glm::mat4 modelMatrix;
	  glm::mat4 normalMatrix;
      };
      struct DrawAnim3D {
	  DrawAnim3D() {}
	  DrawAnim3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix);
	  Resource::Model model;
	  glm::mat4 modelMatrix;
	  glm::mat4 normalMatrix;
	  glm::mat4 bones[Resource::MAX_BONES];
      };

      struct DrawCall {
	  DrawMode mode;
	  Draw2D d2D;
	  Draw3D d3D;
	  DrawAnim3D d3DAnim;
      };

      DrawMode currentDrawMode = DrawMode::d2D;
      unsigned int currentDraw = 0;
      DrawCall drawCalls[MAX_DRAWS];

      glm::mat4 perInstance2DModel[Resource::MAX_2D_BATCH];
      glm::vec4 perInstance2DTexOffset[Resource::MAX_2D_BATCH];
      GLuint model2DSSBO;
      GLuint texOffset2DSSBO;

      glm::mat4 perInstance3DModel[Resource::MAX_3D_BATCH];
      glm::mat4 perInstance3DNormal[Resource::MAX_3D_BATCH];
      GLuint model3DSSBO;
      GLuint normal3DSSBO;
  };

} // namespace glenv

#endif
