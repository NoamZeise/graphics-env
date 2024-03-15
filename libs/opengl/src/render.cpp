#include "render.h"

#include "ogl_helper.h"
#include "shader.h"
#include "resources/vertex_data.h"
#include "resources/resource_pool.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <graphics/logger.h>
#include <graphics/glm_helper.h>
#include <stdexcept>

glm::vec2 getTargetRes(RenderConfig renderConf, glm::vec2 winRes);

namespace glenv {

  bool RenderGl::LoadOpenGL() {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      return true;
  }
    
  RenderGl::RenderGl(GLFWwindow *window, RenderConfig renderConf) : Render(window, renderConf) {
      glfwMakeContextCurrent(window);

      this->window = window;
      this->renderConf = renderConf;
      this->prevRenderConf = renderConf;
      
      if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	  throw std::runtime_error("failed to load glad");
      LOG("glad loaded");

      glEnable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_CULL_FACE);

      shader3D = new GLShader("shaders/opengl/3D-lighting.vert", "shaders/opengl/blinnphong.frag");
      shader3D->Use();
      glUniform1i(shader3D->Location("image"), 0);

      shader3DAnim = new GLShader("shaders/opengl/3D-lighting-anim.vert",
				  "shaders/opengl/blinnphong.frag");
      shader3DAnim->Use();
      glUniform1i(shader3DAnim->Location("image"), 0);

      flatShader = new GLShader("shaders/opengl/flat.vert", "shaders/opengl/flat.frag");
      flatShader->Use();
      glUniform1i(flatShader->Location("image"), 0);

      finalShader = new GLShader("shaders/opengl/final.vert", "shaders/opengl/final.frag");
      finalShader->Use();
      glUniformMatrix4fv(finalShader->Location("screenTransform"),
			 1, GL_FALSE, &finalTransform[0][0]);

      LOG("shaders loaded");
      
      view2D = glm::mat4(1.0f);
  
      ogl_helper::createShaderStorageBuffer(&model3DSSBO, sizeAndPtr(perInstance3DModel));
      ogl_helper::createShaderStorageBuffer(&normal3DSSBO, sizeAndPtr(perInstance3DNormal));
      ogl_helper::createShaderStorageBuffer(&model2DSSBO, sizeAndPtr(perInstance2DModel));  
      ogl_helper::createShaderStorageBuffer(&texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset));
      LOG("shader buffers created");

      pools = new GLPoolManager();
      defaultPool = CreateResourcePool()->id();
      FramebufferResize();
      setLightingProps(BPLighting());
      LOG("renderer initialized");
  }

  RenderGl::~RenderGl() {
      if(useOffscreenFramebuffer) {
	  delete offscreenFramebuffer;
	  if(offscreenBlitFramebuffer != nullptr)
	      delete offscreenBlitFramebuffer;
      }
      delete shader3D;
      delete shader3DAnim;
      delete flatShader;
      delete finalShader;
      delete pools;
  }

  ResourcePool* RenderGl::CreateResourcePool() {
      int i = pools->NextPoolIndex();
      GLResourcePool* p = new GLResourcePool(Resource::Pool(i), renderConf, pools);   
      return pools->AddPool(p, i);
  }
  void RenderGl::DestroyResourcePool(Resource::Pool pool) {
      pools->DeletePool(pool);
  }

  ResourcePool* RenderGl::pool(Resource::Pool pool) {
      _throwIfPoolInvaid(pool);
      return pools->get(pool);
  }
 
  void RenderGl::LoadResourcesToGPU(Resource::Pool pool) {
      _throwIfPoolInvaid(pool);
      pools->get(pool)->loadGpu();
  }

  RenderGl::Draw2D::Draw2D(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset) {
      this->tex = tex;
      this->model = model;
      this->colour = colour;
      this->texOffset = texOffset;
  }

  RenderGl::Draw3D::Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix) {
      this->model = model;
      this->modelMatrix = modelMatrix;
      this->normalMatrix = normalMatrix;
  }

  RenderGl::DrawAnim3D::DrawAnim3D(Resource::Model model, glm::mat4 modelMatrix,
				   glm::mat4 normalMatrix) {
      this->model = model;
      this->modelMatrix = modelMatrix;
      this->normalMatrix = normalMatrix;
  }

  void RenderGl::setVPshader(GLShader *shader) {
      shader->Use();
      glUniformMatrix4fv(shader->Location("projection"), 1, GL_FALSE,
			 &proj3D[0][0]);
      glUniformMatrix4fv(shader->Location("view"), 1, GL_FALSE,
			 &view3D[0][0]);
      glUniform4fv(shader->Location("lighting.camPos"), 1,
		   &lighting.camPos[0]);
      glm::vec4 colourWhite = glm::vec4(1);
      glUniform4fv(shader->Location("spriteColour"), 1, &colourWhite[0]);
      glUniform1i(shader->Location("enableTex"), GL_TRUE);
  }

  void RenderGl::setLightingShader(GLShader *shader) {
      shader->Use();
      glUniform4fv(shader->Location("lighting.ambient"), 1,
		   &lighting.ambient[0]);
      glUniform4fv(shader->Location("lighting.diffuse"), 1,
		   &lighting.diffuse[0]);
      glUniform4fv(shader->Location("lighting.specular"), 1,
		   &lighting.specular[0]);
      glUniform4fv(shader->Location("lighting.direction"), 1,
		   &lighting.direction[0]);
  }

  void RenderGl::setShaderForMode(DrawMode mode, unsigned int i) {
      switch(mode) {
      case DrawMode::d2D:
	  flatShader->Use();
	  glUniformMatrix4fv(flatShader->Location("projection"), 1,
			     GL_FALSE, &proj2D[0][0]);
	  glUniformMatrix4fv(flatShader->Location("view"), 1, GL_FALSE, &view2D[0][0]);
	  glUniform1i(flatShader->Location("enableTex"), GL_TRUE);
	  break;
      case DrawMode::d3D:
	  setVPshader(shader3D);
	  break;
      case DrawMode::d3DAnim:
	  setVPshader(shader3DAnim);
	  glUniformMatrix4fv(shader3DAnim->Location("bones"), Resource::MAX_BONES, GL_FALSE,
			     &drawCalls[i].d3DAnim.bones[0][0][0]);
	  glUniformMatrix4fv(shader3DAnim->Location("model"), 1, GL_FALSE,
			     &drawCalls[i].d3DAnim.modelMatrix[0][0]);
	  glUniformMatrix4fv(shader3DAnim->Location("normal"), 1, GL_FALSE,
			     &drawCalls[i].d3DAnim.normalMatrix[0][0]);
	  break;
      }
  }

#define DRAW_BATCH()							\
  if(drawCount > 0) {							\
      switch(currentMode) {						\
      case DrawMode::d2D:						\
	  draw2DBatch(drawCount, currentTexture, currentColour);	\
	  break;							\
      case DrawMode::d3D:						\
	  draw3DBatch(drawCount, currentModel);				\
	  break;							\
      default:								\
	  throw std::runtime_error("ogl_DRAW_BATCH unknown mode to batch draw!"); \
      }									\
  }

  void RenderGl::EndDraw(std::atomic<bool>& submit) {
      glm::vec2 targetResolution = getTargetRes(renderConf, windowResolution);
      if(useOffscreenFramebuffer) {
	  glBindFramebuffer(GL_FRAMEBUFFER, offscreenFramebuffer->id());
	  glEnable(GL_DEPTH_TEST);
	  glViewport(0, 0, (GLsizei)targetResolution.x, (GLsizei)targetResolution.y);
      }
      glClearColor(renderConf.clear_colour[0],
		   renderConf.clear_colour[1],
		   renderConf.clear_colour[2], 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      if(currentDraw == 0) {
	  glfwSwapBuffers(window);
	  submit = true;
	  return;
      }

      DrawMode currentMode;
      Resource::Texture currentTexture;
      glm::vec4 currentColour;
      Resource::Model currentModel;
      int drawCount = 0;
      for(unsigned int i = 0; i < currentDraw; i++) {
	  if(i == 0 || currentMode != drawCalls[i].mode || currentMode == DrawMode::d3DAnim) {
	      DRAW_BATCH();
	      drawCount = 0;
	      currentMode = drawCalls[i].mode;
	      setShaderForMode(currentMode, i);
	  }
	  switch(currentMode) {
	  case DrawMode::d2D:
	      if((drawCount > 0 &&
		 (currentTexture != drawCalls[i].d2D.tex ||
		  currentColour != drawCalls[i].d2D.colour)) ||
		  drawCount == Resource::MAX_2D_BATCH) {
		  draw2DBatch(drawCount, currentTexture, currentColour);
		  drawCount = 0;
	      }
	      currentTexture = drawCalls[i].d2D.tex;
	      currentColour = drawCalls[i].d2D.colour;
	      perInstance2DModel[drawCount] = drawCalls[i].d2D.model;
	      perInstance2DTexOffset[drawCount] = drawCalls[i].d2D.texOffset;
	      drawCount++;
	      break;
	  case DrawMode::d3D:
	      if((drawCount > 0 && currentModel != drawCalls[i].d3D.model) ||
		 drawCount == Resource::MAX_3D_BATCH) {
		  draw3DBatch(drawCount, currentModel);
		  drawCount = 0;
	      }
	      currentModel = drawCalls[i].d3D.model;
	      perInstance3DModel[drawCount] = drawCalls[i].d3D.modelMatrix;
	      perInstance3DNormal[drawCount] = drawCalls[i].d3D.normalMatrix;
	      drawCount++;
	      break;

	  case DrawMode::d3DAnim:
	      currentModel = drawCalls[i].d3DAnim.model;
	      draw3DAnim(currentModel);
	      drawCount = 0;
	      break;
	  }
      }
      
      DRAW_BATCH();

      if(useOffscreenFramebuffer) {
	  if(renderConf.multisampling) {
	      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, offscreenBlitFramebuffer->id());
	      glBindFramebuffer(GL_READ_FRAMEBUFFER, offscreenFramebuffer->id());
	      glDrawBuffer(GL_BACK);
	      glBlitFramebuffer(0, 0,
				(GLsizei)targetResolution.x,
				(GLsizei)targetResolution.y,
				0, 0,
				(GLsizei)targetResolution.x,
				(GLsizei)targetResolution.y,
				GL_COLOR_BUFFER_BIT, GL_LINEAR);
	  }
	  glBindFramebuffer(GL_FRAMEBUFFER, 0);
	  glClearColor(renderConf.scaled_border_colour[0],
		       renderConf.scaled_border_colour[1],
		       renderConf.scaled_border_colour[2], 1.0f);
	  glClear(GL_COLOR_BUFFER_BIT);
	  glViewport(0, 0, (GLsizei)windowResolution.x, (GLsizei)windowResolution.y);
	  
	  finalShader->Use();
	  glDisable(GL_DEPTH_TEST);
	  glBindTexture(GL_TEXTURE_2D, renderConf.multisampling ?
			offscreenBlitFramebuffer->textureId(0) :
			offscreenFramebuffer->textureId(0));
	  glDrawArrays(GL_TRIANGLES, 0, 3);
      }
      
      glfwSwapBuffers(window);
      inDraw = false;
      currentDraw = 0;
      submit = true;
  }

  void RenderGl::draw2DBatch(int drawCount, Resource::Texture texture, glm::vec4 currentColour) {
      if(!_poolInUse(texture.pool)) {
	  LOG_ERROR("Tried Drawing with pool that is not in use");
	  return;
      }
      glUniform4fv(flatShader->Location("spriteColour"), 1, &currentColour[0]);

      ogl_helper::shaderStorageBufferData(model2DSSBO, sizeAndPtr(perInstance2DModel), 4);
      ogl_helper::shaderStorageBufferData(texOffset2DSSBO, sizeAndPtr(perInstance2DTexOffset), 5);
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, pools->get(texture.pool)->texLoader->getViewIndex(texture));
      pools->get(texture.pool)->modelLoader->DrawQuad(drawCount);
  }

  void RenderGl::draw3DBatch(int drawCount, Resource::Model model) {
      if(!_poolInUse(model.pool)) {
	  LOG_ERROR("Tried Drawing with pool that is not in use");
	  return;
      }
      ogl_helper::shaderStorageBufferData(model3DSSBO, sizeAndPtr(perInstance3DModel), 2);
      ogl_helper::shaderStorageBufferData(normal3DSSBO, sizeAndPtr(perInstance3DNormal), 3);
      pools->get(model.pool.ID)->modelLoader->DrawModelInstanced(
	      model, drawCount,
	      shader3D->Location("spriteColour"), shader3D->Location("enableTex"));
  }

  bool RenderGl::_validPool(Resource::Pool pool) {
      if(!pools->ValidPool(pool)) {
	  LOG_ERROR("Passed Pool does not exist."
		    " It has either been destroyed or was never created.");
	  return false;
      }
      return true;
  }

  bool RenderGl::_poolInUse(Resource::Pool pool) {
      return _validPool(pool) && pools->get(pool)->usingGPUResources;
  }

  void RenderGl::_throwIfPoolInvaid(Resource::Pool pool) {
      if(!_validPool(pool))
	  throw std::runtime_error("Tried to load resource "
				   "with a pool that does not exist");
  }

  void RenderGl::draw3DAnim(Resource::Model model) {
      if(!_poolInUse(model.pool)) {
	  LOG_ERROR("tried to draw string with pool that is not currently in use!");
	  return;
      }
      pools->get(model.pool)->modelLoader->DrawModel(
	      model, shader3DAnim->Location("spriteColour"), shader3DAnim->Location("enableTex"));
  }

  void RenderGl::DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMat) {
      if(currentDraw < MAX_DRAWS) {
	  currentDrawMode = DrawMode::d3D;
	  drawCalls[currentDraw].mode = DrawMode::d3D;
	  drawCalls[currentDraw++].d3D = Draw3D(model, modelMatrix, normalMat);
      }
  }   

  void RenderGl::DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix,
			       glm::mat4 normalMatrix,
			       Resource::ModelAnimation *animation) {
      if(currentDraw < MAX_DRAWS) {
	  currentDrawMode = DrawMode::d3DAnim;
	  drawCalls[currentDraw].mode = DrawMode::d3DAnim;
	  drawCalls[currentDraw].d3DAnim = DrawAnim3D(model, modelMatrix, normalMatrix);
	  std::vector<glm::mat4>* bones = animation->getCurrentBones();
	  for(int i = 0; i < Resource::MAX_BONES && i < bones->size(); i++)
	      drawCalls[currentDraw].d3DAnim.bones[i] = bones->at(i);
	  currentDraw++;
      }
  }

  void RenderGl::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix,
			  glm::vec4 colour, glm::vec4 texOffset) {
      if(currentDraw < MAX_DRAWS) {
	  currentDrawMode = DrawMode::d3D;
	  drawCalls[currentDraw].mode = DrawMode::d2D;
	  drawCalls[currentDraw++].d2D = Draw2D(texture, modelMatrix, colour, texOffset);
      }
  }

  void RenderGl::DrawString(Resource::Font font, std::string text, glm::vec2 position,
			    float size, float depth, glm::vec4 colour, float rotate) {
      if(!_poolInUse(font.pool)) {
	  LOG_ERROR("tried to draw string with pool that is not currently in use!");
	  return;
      }
      auto draws = pools->get(font.pool)->fontLoader->DrawString(
	      font, text, position, size, depth, colour, rotate);
      for(const auto &draw: draws) 
	  DrawQuad(draw.tex, draw.model, draw.colour, draw.texOffset);
  }

  void RenderGl::FramebufferResize() {
      glfwSwapInterval(renderConf.vsync ? 1 : 0);
      int width, height;
      glfwGetFramebufferSize(window, &width, &height);
      windowResolution = glm::vec2((float)width, (float)height);
      glViewport(0, 0, width, height);
      LOG("resizing framebuffer, window width: " << width << "  height:" << height);
      glm::vec2 targetResolution = getTargetRes(renderConf, windowResolution);

      if(renderConf.multisampling) 
	  glEnable(GL_MULTISAMPLE);
      else
	  glDisable(GL_MULTISAMPLE);

      if(useOffscreenFramebuffer) {
	  if(renderConf.multisampling)
	      glGetIntegerv(GL_MAX_SAMPLES, &msaaSamples);
	  else
	      msaaSamples = 1;
	  if(offscreenFramebuffer != nullptr)
	      delete offscreenFramebuffer;
	  offscreenFramebuffer = new GlFramebuffer(
		  (GLsizei)targetResolution.x, (GLsizei)targetResolution.y, msaaSamples, {
		      GlFramebuffer::Attachment(
			      GlFramebuffer::Attachment::Position::color0,
			      renderConf.multisampling ?
			      GlFramebuffer::AttachmentType::renderbuffer :
			      GlFramebuffer::AttachmentType::texture2D,
			      GL_RGB),
		       GlFramebuffer::Attachment(
		            GlFramebuffer::Attachment::Position::depthStencil,
		            GlFramebuffer::AttachmentType::renderbuffer,
		            GL_DEPTH24_STENCIL8),
		  });
	  if(renderConf.multisampling) {
	      offscreenBlitFramebuffer = new GlFramebuffer(
		      (GLsizei)targetResolution.x, (GLsizei)targetResolution.y, 1, {
			  GlFramebuffer::Attachment(
				  GlFramebuffer::Attachment::Position::color0,
				  GlFramebuffer::AttachmentType::texture2D,
				  GL_RGB)});
	  }
	  finalShader->Use();
	  finalTransform = glmhelper::calcFinalOffset(targetResolution, windowResolution);
	  glUniformMatrix4fv(finalShader->Location("screenTransform"),
			     1, GL_FALSE, &finalTransform[0][0]);
      }
      prevRenderConf = renderConf;
  }

  void RenderGl::set3DViewMat(glm::mat4 view, glm::vec4 camPos) {
      view3D = view;
      lighting.camPos = camPos;
  }

  void RenderGl::set2DViewMat(glm::mat4 view) {
      view2D = view;
  }

  void RenderGl::set3DProjMat(glm::mat4 proj) {
      proj3D = proj;
  }

  void RenderGl::set2DProjMat(glm::mat4 proj) {
      proj2D = proj;
  }

  void RenderGl::setRenderConf(RenderConfig renderConf) {
      this->renderConf = renderConf;
      FramebufferResize();
  }
  
  RenderConfig RenderGl::getRenderConf() {
      return renderConf;
  }

  void RenderGl::setLightingProps(BPLighting lighting) {
      this->lighting = lighting;
      setLightingShader(shader3D);
      setLightingShader(shader3DAnim);
  }
  
  glm::vec2 RenderGl::offscreenSize() {
      return
	  renderConf.target_resolution[0] == 0.0 ||
	  renderConf.target_resolution[1] == 0.0 ?
	  glm::vec2(windowResolution.x, windowResolution.y) :
	  glm::vec2(renderConf.target_resolution[0], renderConf.target_resolution[1]);
  }
  
}//namespace

glm::vec2 getTargetRes(RenderConfig renderConf, glm::vec2 winRes) {
    glm::vec2 targetResolution(renderConf.target_resolution[0],
			       renderConf.target_resolution[1]);
    if(targetResolution.x == 0.0 || targetResolution.y == 0.0)
	targetResolution = winRes;
    return targetResolution;
}
