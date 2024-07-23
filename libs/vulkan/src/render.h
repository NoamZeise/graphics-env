#ifndef RENDER_H
#define RENDER_H

#define VOLK_IMPLIMENTATION
#include <volk.h>
#include <GLFW/glfw3.h>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>

#include <graphics/render.h>
#include <graphics/shader_structs.h>

#include "vulkan_manager.h"
#include "swapchain.h"
#include "frame.h"
#include "renderpass.h"
#include "pipeline.h"
#include "shader_structs.h"
#include <atomic>
#include <vector>

class PoolManagerVk;
class ShaderPoolVk;
class ShaderSet;

namespace vkenv {

const size_t MAX_ANIMATIONS_PER_FRAME = 10;

  class RenderVk : public Render {
  public:
      /// Try and load Vulkan functions from the installed driver.
      /// Returns whether the operation succeeded or not.
      /// If the operation failed, don't try to create an object of type Render.
      /// This will be called by Render automatically if not called before Render is created.
      static bool LoadVulkan();

      RenderVk(GLFWwindow *window, RenderConfig renderConf, shader::PipelineSetup pipelineSetup);
      ~RenderVk();

      // Resource Pools
      ResourcePool* CreateResourcePool() override;
      void DestroyResourcePool(Resource::Pool pool) override;
      void setResourcePoolInUse(Resource::Pool pool, bool usePool) override;
      ResourcePool* pool(Resource::Pool pool) override;
      void LoadResourcesToGPU(Resource::Pool pool) override;

      // Shader Pools
      ShaderPool* CreateShaderPool();
      void DestroyShaderPool(ShaderPool* pool);

      // Pipeline
      Pipeline* CreatePipeline(
	      PipelineInput input,
	      std::vector<char> vertexCode,
	      std::vector<char> fragmentCode);
      
      void UseLoadedResources() override;

      // warning: switching between models that are in different pools often is slow
      void DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix) override;
      void DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix,
			 Resource::ModelAnimation *animation) override;
      void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour,
		    glm::vec4 texOffset) override;
      void DrawString(Resource::Font font, std::string text, glm::vec2 position, float size,
		      float depth, glm::vec4 colour, float rotate) override;
      void EndDraw(std::atomic<bool> &submit) override;

      void FramebufferResize() override;
      
      void set3DViewMat(glm::mat4 view, glm::vec4 camPos) override;
      void set2DViewMat(glm::mat4 view) override;
      void set3DProjMat(glm::mat4 proj) override;
      void set2DProjMat(glm::mat4 proj) override;
      void setLightingProps(BPLighting lighting) override;

      void setTime(float time) {
	  timeData.time = time;
      }
    
  private:
      enum class RenderState {
	  Draw2D,
	  Draw3D,
	  DrawAnim3D,
      };
      
      void _initFrameResources();
      void _destroyFrameResources();
      void _startDraw();
      void _begin(RenderState state);
      void _store3DsetData();
      void _store2DsetData();
      void _resize();
      void _drawBatch();
      void _bindModelPool(Resource::Model model);
      bool _validPool(Resource::Pool pool);
      bool _poolInUse(Resource::Pool pool);
      void _throwIfPoolInvaid(Resource::Pool pool);
      std::vector<Resource::Texture> getActiveTextures(float* getMinMipmap);
      
      
      bool _framebufferResized = false;
      bool _frameResourcesCreated = false;     
  
      VulkanManager* manager = nullptr;
      uint32_t frameIndex = 0;
      const uint32_t MAX_CONCURRENT_FRAMES = 2;
      Frame** frames;      

      VkFormat offscreenDepthFormat;
      VkFormat prevSwapchainFormat = VK_FORMAT_UNDEFINED;
      VkSampleCountFlagBits prevSampleCount = VK_SAMPLE_COUNT_1_BIT;
      Swapchain *swapchain = nullptr;
      uint32_t swapchainFrameIndex = 0;
      uint32_t swapchainFrameCount = 0;
      VkCommandBuffer currentCommandBuffer = VK_NULL_HANDLE;

      RenderPass* offscreenRenderPass = nullptr;
      RenderPass* finalRenderPass = nullptr;
      bool usingFinalRenderPass = false;

      PipelineOld _pipeline3D;
      PipelineOld _pipelineAnim3D;
      PipelineOld _pipeline2D;
      PipelineOld _pipelineFinal;
      
      ShaderPool* mainShaderPool;      
      ShaderSet* perFrame2DSet;
      shaderStructs::Frag2DData perFrame2DFragData[Resource::MAX_2D_BATCH];
      ShaderSet* lightingSet;
      BPLighting lightingData;
      ShaderSet* textureSet;
      ShaderSet* boneSet;
      size_t currentBonesDynamicIndex;
      ShaderSet* emptySet;
      ShaderSet* vp3dSet;
      shaderStructs::timeUbo timeData;
      shaderStructs::viewProjection VP3DData;
      ShaderSet* vp2dSet;
      shaderStructs::viewProjection VP2DData;
      ShaderSet* perFrame3dSet;
      shaderStructs::PerFrame3D perFrame3DData[Resource::MAX_3D_BATCH];
      ShaderSet* perFrame2dVertSet;
      glm::mat4 perFrame2DVertData[Resource::MAX_2D_BATCH];
      ShaderSet* offscreenTransformSet;
      ShaderSet* offscreenTexSet;

      std::vector<ShaderPoolVk*> shaderPools;

      Resource::Pool defaultResourcePool;
      ResourcePoolVk* framebufferResourcePool;
      PoolManagerVk* pools;

      bool _begunDraw = false;
      RenderState _renderState;

      unsigned int _modelRuns = 0;
      unsigned int _current3DInstanceIndex = 0;
      Resource::Model _currentModel;
      Resource::Texture _currentTexture;
      glm::vec4 _currentTexOffset = glm::vec4(0, 0, 1, 1);
      glm::vec4 _currentColour = glm::vec4(1, 1, 1, 1);

      unsigned int _instance2Druns = 0;
      unsigned int _current2DInstanceIndex = 0;

      Resource::Pool currentModelPool;
  };

} //namespace

#endif
