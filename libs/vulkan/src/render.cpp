#include "render.h"

#include "parts/pipeline.h"
#include "parts/descriptors.h"
#include "parts/images.h"
#include "pipeline.h"
#include "pipeline_data.h"
#include "resources/resource_pool.h"
#include "vkhelper.h"
#include "logger.h"

#include <resource-loaders/pool_manager.h>
#include <graphics/glm_helper.h>
#include <graphics/shader.h>

#include <GLFW/glfw3.h>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>


namespace vkenv {

bool RenderVk::LoadVulkan() {
    if(volkInitialize() != VK_SUCCESS) {
      return false;
    }
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    return true;
}

void checkVolk() {
    if(volkGetInstanceVersion() == 0) {
	// if user hasn't checked for vulkan support, try loading vulkan first.
	if(!RenderVk::LoadVulkan())
	    throw std::runtime_error("Vulkan has not been loaded! Either the "
				     "graphics devices does not support vulkan, "
				     "or Vulkan drivers aren't installed");		    
    }
}

VkFormat getDepthBufferFormat(VkPhysicalDevice physicalDevice) {
    return vkhelper::findSupportedFormat(
	    physicalDevice,
	    {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
	    VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

  RenderVk::RenderVk(GLFWwindow *window, RenderConfig renderConf, shader::PipelineSetup pipelineSetup) : Render(window, renderConf, pipelineSetup) {
    checkVolk();
    EnabledFeatures features;
    features.sampleRateShading = renderConf.sample_shading;
    manager = new VulkanManager(window, features);
    offscreenDepthFormat = getDepthBufferFormat(manager->deviceState.physicalDevice);
    
    frames = new Frame*[MAX_CONCURRENT_FRAMES];
    for(int i = 0; i < MAX_CONCURRENT_FRAMES; i++)
	frames[i] = new Frame(manager->deviceState.device,
			      manager->deviceState.queue.graphicsPresentFamilyIndex);
    pools = new PoolManagerVk;
    defaultPool = CreateResourcePool()->id();
}
  
RenderVk::~RenderVk() {
    vkDeviceWaitIdle(manager->deviceState.device);

    _destroyFrameResources();
    delete pools;
    if(offscreenRenderPass != nullptr || finalRenderPass != nullptr) {
	delete offscreenRenderPass;
	if(usingFinalRenderPass)
	    delete finalRenderPass;
	vkFreeMemory(manager->deviceState.device, framebufferMemory, VK_NULL_HANDLE);
    }
    if(offscreenSamplerCreated)
	vkDestroySampler(manager->deviceState.device, _offscreenTextureSampler, nullptr);
    if(textureSamplerCreated)
	vkDestroySampler(manager->deviceState.device, textureSampler, nullptr);
    if(swapchain != nullptr)
	delete swapchain;
    for(int i = 0; i < MAX_CONCURRENT_FRAMES; i++)
	delete frames[i];
    delete[] frames;
    delete manager;
}

bool swapchainRecreationRequired(VkResult result) {
    return result == VK_SUBOPTIMAL_KHR ||
	result == VK_ERROR_OUT_OF_DATE_KHR;
}

  void RenderVk::_loadActiveTextures() {
      // Load textures across all active resource pools
      // into the textureViews descriptor set data
      if(pools->PoolCount() < 1)
	  throw std::runtime_error("Must have at least 1 pool.");
      ResourcePoolVk *pool = pools->get(0);
      VkImageView validView;
      bool foundValidView = false;
      bool checkedAllPools = false;
      for(int i = 0, pI = 0, texI = 0; i < Resource::MAX_TEXTURES_SUPPORTED; i++) {
	  if(pool == nullptr)
	      goto next_pool;
	  if(!pool->UseGPUResources) {
	      pool->usingGPUResources = false;
	      goto next_pool;
	  }
	  pool->usingGPUResources = true;
	  if(texI < pool->texLoader->getImageCount()) {
	      textureViews[i] = pool->texLoader->getImageViewSetIndex(texI++, i);
	      if (!foundValidView) {
		  foundValidView = true;
		  validView = textureViews[i];
	      }              
	  } else {
	      goto next_pool;
	  }
	  continue;
      next_pool:
	  if(pools->PoolCount() > pI + 1) {
	      pool = pools->get(++pI);
	      texI = 0;
	      i--;
	  } else {
	      checkedAllPools = true;
	      if(foundValidView)
		  textureViews[i] = validView;
	      else //TODO: change so we dont require a texture
		  throw std::runtime_error("No textures were loaded. "
					   "At least 1 Texture must be loaded");
	  }
      }
      if(!checkedAllPools) {
	  LOG_ERROR("Ran out of texture slots in shader! current limit: "
		    << Resource::MAX_TEXTURES_SUPPORTED);
      }
  }

  
  void RenderVk::_initFrameResources() {
      LOG("Creating Swapchain");
      
      if(_frameResourcesCreated)
	  _destroyFrameResources();
	    
      int winWidth, winHeight;
      glfwGetFramebufferSize(window, &winWidth, &winHeight);
      while(winWidth == 0 || winHeight == 0) {
	  glfwGetFramebufferSize(window, &winWidth, &winHeight);
	  glfwWaitEvents();
      }
      windowResolution = glm::vec2((float)winWidth, (float)winHeight);
      glm::vec2 target = offscreenSize();
            
      bool useFinalRenderpass = renderConf.forceFinalBuffer ||
	  target != glm::vec2(winWidth, winHeight);
      
      VkExtent2D offscreenBufferExtent = {(uint32_t)target.x, (uint32_t)target.y};
      VkExtent2D swapchainExtent = {(uint32_t)winWidth, (uint32_t)winHeight};
      
      if(swapchain == nullptr)
	  swapchain = new Swapchain(
		  manager->deviceState.device,
		  manager->deviceState.physicalDevice,
		  manager->windowSurface, swapchainExtent, renderConf);
      else
	  swapchain->RecreateSwapchain(swapchainExtent, renderConf);

      LOG("Creating Render Passes");

      VkFormat swapchainFormat = swapchain->getFormat();
      VkSampleCountFlagBits sampleCount = vkhelper::getMaxSupportedMsaaSamples(
	      manager->deviceState.device,
	      manager->deviceState.physicalDevice);
      if(!renderConf.multisampling)
	  sampleCount = VK_SAMPLE_COUNT_1_BIT;
      
      if(swapchainFormat != prevSwapchainFormat || sampleCount != prevSampleCount ||
	 useFinalRenderpass != usingFinalRenderPass) {
	  if(offscreenRenderPass != nullptr) {
	      delete offscreenRenderPass;
	      if(usingFinalRenderPass) {
		  usingFinalRenderPass = false;
		  delete finalRenderPass;
	      }
	  }
	  auto offscreenFinalAttachUse = useFinalRenderpass ?
	      AttachmentUse::ShaderRead : AttachmentUse::PresentSrc;
	  std::vector<AttachmentDesc> offscreenAttachments;
	  if(renderConf.multisampling) {
	      offscreenAttachments.push_back(
		      AttachmentDesc(1, AttachmentType::Colour,
				     AttachmentUse::TransientAttachment,
				     sampleCount, swapchainFormat));
	      offscreenAttachments.push_back(
		      AttachmentDesc(0, AttachmentType::Resolve,
				     offscreenFinalAttachUse,
				     VK_SAMPLE_COUNT_1_BIT, swapchainFormat));
	  } else
	      offscreenAttachments.push_back(
		      AttachmentDesc(0, AttachmentType::Colour,
				     offscreenFinalAttachUse,
				     VK_SAMPLE_COUNT_1_BIT, swapchainFormat));
	  if(renderConf.useDepthTest) {
	      offscreenAttachments.push_back(
		      AttachmentDesc(renderConf.multisampling ? 2 : 1, AttachmentType::Depth,
				     AttachmentUse::Attachment,
				     sampleCount, offscreenDepthFormat));
	  }

	  offscreenRenderPass = new RenderPass(manager->deviceState.device, offscreenAttachments,
					       renderConf.clear_colour);
	  if(useFinalRenderpass) {
	      usingFinalRenderPass = true;
	      finalRenderPass =
		  new RenderPass(manager->deviceState.device,
				 { AttachmentDesc(0, AttachmentType::Colour,
						  AttachmentUse::PresentSrc,
						  VK_SAMPLE_COUNT_1_BIT, swapchainFormat)},
				 renderConf.scaled_border_colour);
	  }
      }
      
      prevSwapchainFormat = swapchainFormat;
      prevSampleCount = sampleCount;

      std::vector<VkImage>* swapchainImages = swapchain->getSwapchainImages();
      swapchainFrameCount = swapchainImages->size();

      LOG("Creating Framebuffers");

      //TODO: less unnessecary recreation (ie offscreen extent not changing?)
      VkDeviceSize attachmentMemorySize = 0;
      uint32_t attachmentMemoryFlags = 0;
      offscreenRenderPass->createFramebufferImages(
	      useFinalRenderpass ? nullptr : swapchainImages, offscreenBufferExtent,
	      &attachmentMemorySize, &attachmentMemoryFlags);

      if(useFinalRenderpass)
	  finalRenderPass->createFramebufferImages(
		  swapchainImages, swapchainExtent,
		  &attachmentMemorySize, &attachmentMemoryFlags);
      if(attachmentMemorySize > 0) {
	  vkFreeMemory(manager->deviceState.device, framebufferMemory, VK_NULL_HANDLE);
	  checkResultAndThrow(
		  vkhelper::allocateMemory(
			  manager->deviceState.device,
			  manager->deviceState.physicalDevice,
			  attachmentMemorySize,
			  &framebufferMemory,
			  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			  attachmentMemoryFlags),
		  "Render Error: Failed to Allocate Memory for Framebuffer Images");
      }
      
      offscreenRenderPass->createFramebuffers(framebufferMemory);
      if(useFinalRenderpass)
	  finalRenderPass->createFramebuffers(framebufferMemory);

      LOG("Swapchain Image Count: " << swapchainImages->size());
            
      
      LOG("Creating Texture Sampler");

      // Add textures from resource pools into texture indexes
      _loadActiveTextures();
      
      float minMipmapLevel = 100000.0f;
      for(int i = 0; i < pools->PoolCount(); i++) {
	  ResourcePoolVk* p = pools->get(i);
	  if(p == nullptr)
	      continue;
	  if(p->usingGPUResources) {
	      float n = p->texLoader->getMinMipmapLevel();
	      if(n < minMipmapLevel)
		  minMipmapLevel = n;
	  }
      }
    
      if(textureSamplerCreated) {
	  if(prevRenderConf.texture_filter_nearest != renderConf.texture_filter_nearest ||
	     prevTexSamplerMinMipmap != minMipmapLevel) {
	      textureSamplerCreated = false;
	      vkDestroySampler(manager->deviceState.device, textureSampler, nullptr);
	  }
      }
    
      if(!textureSamplerCreated) {	  
	  checkResultAndThrow(
		  part::create::TextureSampler(
			  manager->deviceState.device,
			  manager->deviceState.physicalDevice,
			  &textureSampler,
			  minMipmapLevel,
			  manager->deviceState.features.samplerAnisotropy,
			  renderConf.texture_filter_nearest ?
			  VK_FILTER_NEAREST : VK_FILTER_LINEAR,
			  VK_SAMPLER_ADDRESS_MODE_REPEAT),
		  "Failed to create texture sampler");
	  prevTexSamplerMinMipmap = minMipmapLevel;
	  textureSamplerCreated = true;
      }


      LOG("Creating Descriptor Sets");

      int descriptorSizes = MAX_CONCURRENT_FRAMES;

      mainShaderPool = CreateShaderPool();
      
      lightingSet = mainShaderPool->CreateSet(stageflag::frag);
      lightingSet->addUniformBuffer(0, sizeof(BPLighting));

      perFrame2DSet = mainShaderPool->CreateSet(stageflag::frag);
      perFrame2DSet->addStorageBuffer(
	      0, sizeof(shaderStructs::Frag2DData) * Resource::MAX_2D_BATCH);
      

      textureSet = mainShaderPool->CreateSet(stageflag::frag);
      textureSet->addTextureSamplers(
	      0, TextureSampler(renderConf.texture_filter_nearest ?
				TextureSampler::filter::nearest : TextureSampler::filter::linear,
				TextureSampler::address_mode::repeat,
				minMipmapLevel));

      std::vector<Resource::Texture> allTextures;      
      for(int i = 0; i < pools->PoolCount(); i++) {
	  std::vector<Resource::Texture> texs = pools->get(i)->texLoader->getTextures();
	  for(int j = 0; j < texs.size(); j++) {
	      //      pools->get(i)->texLoader->setIndex(texs[j], allTextures.size());
	      allTextures.push_back(texs[j]);
	  }	  
      }
      //      textureSet->addTextures(1, allTextures);
      
      /*		
	descriptor::Set texture_Set("textures", descriptor::ShaderStage::Fragment);
	texture_Set.AddSamplerDescriptor("sampler", 1, &textureSampler);
	texture_Set.AddImageViewDescriptor("views", descriptor::Type::SampledImage,
					   Resource::MAX_TEXTURES_SUPPORTED,
					   textureViews);
	textures = new DescSet(texture_Set, descriptorSizes, manager->deviceState.device);

	std::vector<VkImageView> offscreenViews;
	if(useFinalRenderpass) {
	
	  if(!offscreenSamplerCreated) {
	      _offscreenTextureSampler = vkhelper::createTextureSampler(
		      manager->deviceState.device,
		      manager->deviceState.physicalDevice, 1.0f,
		      false,
		      true,
		      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	      offscreenSamplerCreated = true;
	  }
	  
	  offscreenViews = offscreenRenderPass->getAttachmentViews(0);
	  
	  if(offscreenViews.size() == 0)
	      throw std::runtime_error(
		      "Nothing returned for offscreen texture views");
		      
	  descriptor::Set offscreen_Set("offscreen texture", descriptor::ShaderStage::Fragment);
	  
	  offscreen_Set.AddSamplerDescriptor("sampler", 1, &_offscreenTextureSampler);
	  
	  offscreen_Set.AddImageViewDescriptor("frame", descriptor::Type::SampledImage,
					       1, &offscreenViews[0]);
					       
	  offscreenTex = new DescSet(offscreen_Set, descriptorSizes,
				     manager->deviceState.device);
				     
	  descriptorSets.push_back(offscreenTex);
      }
      
      */

      LoadResourcesToGPU(mainShaderPool);

      // can recreate sampler without remaking memory
      //textureSet->updateSampler(0, TextureSampler{});

      
      /// vertex descripor sets
      descriptor::Descriptor viewProjectionBinding(
	      "view projection struct",
	      descriptor::Type::UniformBuffer,
	      sizeof(shaderStructs::viewProjection), 1);
      descriptor::Descriptor timeBinding(
	      "Time Struct", descriptor::Type::UniformBuffer,
	      sizeof(shaderStructs::timeUbo), 1);
      descriptor::Set VP3D_Set("VP3D", descriptor::ShaderStage::Vertex);
      VP3D_Set.AddDescriptor(viewProjectionBinding);
      VP3D_Set.AddDescriptor(timeBinding);
      VP3D = new DescSet(VP3D_Set, descriptorSizes, manager->deviceState.device);
      descriptorSets.push_back(VP3D);
      
      descriptor::Set VP2D_Set("VP2D", descriptor::ShaderStage::Vertex);
      VP2D_Set.AddDescriptor(viewProjectionBinding);
      VP2D = new DescSet(VP2D_Set, descriptorSizes, manager->deviceState.device);
      descriptorSets.push_back(VP2D);

      descriptor::Set PerFrame3D_Set("Per Frame 3D", descriptor::ShaderStage::Vertex);
      PerFrame3D_Set.AddSingleArrayStructDescriptor(
	      "3D Instance Array",
	      descriptor::Type::StorageBuffer,
	      sizeof(shaderStructs::PerFrame3D),
	      Resource::MAX_3D_BATCH);
      perFrame3D = new DescSet(PerFrame3D_Set, descriptorSizes, manager->deviceState.device);
      descriptorSets.push_back(perFrame3D);

      descriptor::Set bones_Set("Bones Animation", descriptor::ShaderStage::Vertex);
      bones_Set.AddDescriptor("bones", descriptor::Type::UniformBufferDynamic,
			      sizeof(shaderStructs::Bones), MAX_ANIMATIONS_PER_FRAME);
      bones = new DescSet(bones_Set, descriptorSizes, manager->deviceState.device);
      descriptorSets.push_back(bones);

      descriptor::Set vert2D_Set("Per Frame 2D Vert", descriptor::ShaderStage::Vertex);
      vert2D_Set.AddSingleArrayStructDescriptor(
	      "vert struct", descriptor::Type::StorageBuffer,
	      sizeof(glm::mat4), Resource::MAX_2D_BATCH);
      perFrame2DVert = new DescSet(vert2D_Set, descriptorSizes, manager->deviceState.device);
      descriptorSets.push_back(perFrame2DVert);
      
      if(useFinalRenderpass) {
	  descriptor::Set offscreenView_Set("Offscreen Transform", descriptor::ShaderStage::Vertex);
	  offscreenView_Set.AddDescriptor("data", descriptor::Type::UniformBuffer,
					  sizeof(glm::mat4), 1);
	  offscreenTransform = new DescSet(
		  offscreenView_Set, descriptorSizes, manager->deviceState.device);
	  descriptorSets.push_back(offscreenTransform);
      }

      // fragment descriptor sets
      
      descriptor::Set texture_Set("textures", descriptor::ShaderStage::Fragment);
      texture_Set.AddSamplerDescriptor("sampler", 1, &textureSampler);
      texture_Set.AddImageViewDescriptor("views", descriptor::Type::SampledImage,
					 Resource::MAX_TEXTURES_SUPPORTED,
					 textureViews);
      textures = new DescSet(texture_Set, descriptorSizes, manager->deviceState.device);
      descriptorSets.push_back(textures);
      
      emptyDS = new DescSet(
	      descriptor::Set("Empty", descriptor::ShaderStage::Vertex),
	      descriptorSizes, manager->deviceState.device);
      descriptorSets.push_back(emptyDS);

      std::vector<VkImageView> offscreenViews; // needs to be in scope for prepareShaderbuffersets
      if(useFinalRenderpass) {
	  if(!offscreenSamplerCreated) {
	      checkResultAndThrow(
		      part::create::TextureSampler(
			      manager->deviceState.device,
			      manager->deviceState.physicalDevice,
			      &_offscreenTextureSampler,
			      1.0f, false,
			      VK_FILTER_NEAREST,
			      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER),
		      "Failed to create texture sampler");
	      offscreenSamplerCreated = true;
	  }
	  offscreenViews = offscreenRenderPass->getAttachmentViews(0);
	  if(offscreenViews.size() == 0)
	      throw std::runtime_error(
		      "Nothing returned for offscreen texture views");
	  descriptor::Set offscreen_Set("offscreen texture", descriptor::ShaderStage::Fragment);
	  offscreen_Set.AddSamplerDescriptor("sampler", 1, &_offscreenTextureSampler);
	  offscreen_Set.AddImageViewDescriptor("frame", descriptor::Type::SampledImage,
					       1, &offscreenViews[0]);
	  offscreenTex = new DescSet(offscreen_Set, descriptorSizes,
				     manager->deviceState.device);
	  descriptorSets.push_back(offscreenTex);
      }
      
      LOG("Creating Descriptor pool and memory for set bindings");
      
      // create descripor pool

      std::vector<DS::DescriptorSet* > sets(descriptorSets.size());
      std::vector<DS::Binding*> bindings;
      for(int i = 0; i < sets.size(); i++) {
	  sets[i] = &descriptorSets[i]->set;
	  for(int j = 0; j < descriptorSets[i]->bindings.size(); j++)
	      bindings.push_back(&descriptorSets[i]->bindings[j]);
      }
     
      part::create::DescriptorPoolAndSet(
	      manager->deviceState.device, &_descPool, sets,
	      descriptorSizes);
      
      // create memory mapped buffer for all descriptor set bindings
      part::create::PrepareShaderBufferSets(
	      manager->deviceState, bindings,
	      &_shaderBuffer, &_shaderMemory);
      
      LOG("Creating Graphics Pipelines");

      // create pipeline for each shader set -> 3D, animated 3D, 2D, and final
      part::create::PipelineConfig pipelineConf;
      pipelineConf.useMultisampling = renderConf.multisampling;
      pipelineConf.msaaSamples = sampleCount;
      pipelineConf.useSampleShading = manager->deviceState.features.sampleRateShading;
      pipelineConf.useDepthTest = renderConf.useDepthTest;
      
      part::create::GraphicsPipeline(
	      manager->deviceState.device, &_pipeline3D,
	      offscreenRenderPass->getRenderPass(),
	      {&VP3D->set, &perFrame3D->set, &emptyDS->set, &textures->set},
	      {(SetVk*)lightingSet},
	      {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(fragPushConstants)}},
	      pipelineSetup.getPath(shader::pipeline::_3D, shader::stage::vert),
	      pipelineSetup.getPath(shader::pipeline::_3D, shader::stage::frag),
	      offscreenBufferExtent,
	      pipeline_inputs::V3D::attributeDescriptions(),
	      pipeline_inputs::V3D::bindingDescriptions(),
	      pipelineConf);
      
      part::create::GraphicsPipeline(
	      manager->deviceState.device, &_pipelineAnim3D,
	      offscreenRenderPass->getRenderPass(),
	      {&VP3D->set, &perFrame3D->set, &bones->set, &textures->set},
	      {(SetVk*)lightingSet},
	      {{VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(fragPushConstants)}},
	      pipelineSetup.getPath(shader::pipeline::anim3D, shader::stage::vert),
	      pipelineSetup.getPath(shader::pipeline::anim3D, shader::stage::frag),
	      offscreenBufferExtent,
	      pipeline_inputs::VAnim3D::attributeDescriptions(),
	      pipeline_inputs::VAnim3D::bindingDescriptions(),
	      pipelineConf);

      part::create::GraphicsPipeline(
	      manager->deviceState.device, &_pipeline2D,
	      offscreenRenderPass->getRenderPass(),
	      {&VP2D->set, &perFrame2DVert->set, &textures->set},
	      {(SetVk*)perFrame2DSet},
	      {},
	      pipelineSetup.getPath(shader::pipeline::_2D, shader::stage::vert),
	      pipelineSetup.getPath(shader::pipeline::_2D, shader::stage::frag),
	      offscreenBufferExtent,
	      pipeline_inputs::V2D::attributeDescriptions(),
	      pipeline_inputs::V2D::bindingDescriptions(),
	      pipelineConf);

      if(useFinalRenderpass) {
	  pipelineConf.useMultisampling = false;
	  pipelineConf.useDepthTest = false;
	  pipelineConf.blendEnabled = false;
	  pipelineConf.cullMode = VK_CULL_MODE_NONE;

	  part::create::GraphicsPipeline(
		  manager->deviceState.device, &_pipelineFinal,
		  finalRenderPass->getRenderPass(),
		  {&offscreenTransform->set, &offscreenTex->set},
		  {},
		  {},
		  pipelineSetup.getPath(shader::pipeline::final, shader::stage::vert),
		  pipelineSetup.getPath(shader::pipeline::final, shader::stage::frag),
		  swapchainExtent, {}, {},
		  pipelineConf);
	  
	  offscreenTransformData = glmhelper::calcFinalOffset(
		  glm::vec2(offscreenBufferExtent.width, offscreenBufferExtent.height),
		  glm::vec2((float)swapchainExtent.width,
			    (float)swapchainExtent.height));
      }
      
      LOG("Finished Creating Frame Resources");
      timeData.time = 0;
      prevRenderConf = renderConf;
      _frameResourcesCreated = true;
  }

  void RenderVk::_destroyFrameResources() {
      if(!_frameResourcesCreated)
	  return;
      LOG("Destroying frame resources");
      LOG("    freeing shader memory");
      vkDestroyBuffer(manager->deviceState.device, _shaderBuffer, nullptr);
      vkFreeMemory(manager->deviceState.device, _shaderMemory, nullptr);
      LOG("    destroying descriptors");

      // move to destructor after desc sets not remade each frame
      DestroyShaderPool(mainShaderPool);
      
      for(int i = 0; i < descriptorSets.size(); i++)
	  delete descriptorSets[i];
      descriptorSets.clear();
      vkDestroyDescriptorPool(manager->deviceState.device, _descPool, nullptr);
      LOG("    destroying Pipelines");
      _pipeline3D.destroy(manager->deviceState.device);
      _pipelineAnim3D.destroy(manager->deviceState.device);
      _pipeline2D.destroy(manager->deviceState.device);
      if(usingFinalRenderPass) {
	  _pipelineFinal.destroy(manager->deviceState.device);
      }
      LOG("    closing pools");
      for(int i = 0; i < pools->PoolCount(); i++)
	  if(pools->get(i) != nullptr)
	      pools->get(i)->usingGPUResources = false;
      _frameResourcesCreated = false;
  }

ResourcePool* RenderVk::CreateResourcePool() {
    int i = pools->NextPoolIndex();
    ResourcePoolVk* p = new ResourcePoolVk(
	    i, pools,
	    manager->deviceState, manager->generalCommandPool, manager->generalCommandBuffer,
	    renderConf);    
    return pools->AddPool(p, i);
}

void RenderVk::DestroyResourcePool(Resource::Pool pool) {
    if(!_validPool(pool))
	return;
    bool reloadResources = false;
    if(pools->get(pool.ID)->usingGPUResources) {
	reloadResources = true;
	vkDeviceWaitIdle(manager->deviceState.device);
	_destroyFrameResources();
    }
    pools->DeletePool(pool);
    if(reloadResources)
	_initFrameResources();
}

  void RenderVk::setResourcePoolInUse(Resource::Pool pool, bool usePool) {
      if(!_validPool(pool))
	  return;
      pools->get(pool)->setUseGPUResources(usePool);
  }

  ResourcePool* RenderVk::pool(Resource::Pool pool) {
      _throwIfPoolInvaid(pool);
      return pools->get(pool);
  }

  bool RenderVk::_validPool(Resource::Pool pool) {
      if(!pools->ValidPool(pool)) {
	  LOG_ERROR("Passed Pool does not exist."
		    " It has either been destroyed or was never created.");
	  return false;
      }
      return true;
  }

bool RenderVk::_poolInUse(Resource::Pool pool) {
    return _validPool(pool) && pools->get(pool)->usingGPUResources;
}

void RenderVk::_throwIfPoolInvaid(Resource::Pool pool) {
    if(!_validPool(pool))
	throw std::runtime_error("Tried to load resource "
				 "with a pool that does not exist");
}

void RenderVk::LoadResourcesToGPU(Resource::Pool pool) {
    _throwIfPoolInvaid(pool);
    bool remakeFrameRes = false;
    if(pools->get(pool)->usingGPUResources) {
      LOG("Loading resources for pool that is currently in use, "
          "so recreating frame resources.");
      vkDeviceWaitIdle(manager->deviceState.device);
      remakeFrameRes = true;
    }
    pools->get(pool)->loadGpu(); 
    if(remakeFrameRes) //remake if pool currently in use was reloaded
	UseLoadedResources();
}

void RenderVk::UseLoadedResources() {
    vkDeviceWaitIdle(manager->deviceState.device);
    if(!_frameResourcesCreated) {
	_initFrameResources();
	return;
    } else {
	_loadActiveTextures();    
	textures->bindings[1].storeImageViews(manager->deviceState.device);
    }
    //TODO : consider mimap levels
    
    // diff pools ->
    
    // check mipmaps?
    // if diff -> recreate sampler.
    //            if sampler recreated -> recreate texture desc set
    
    // diff textures ->
    // recreates textureViews array
    // diff tex views -> diff texture desc set

    //try without caring about mipmaps
}

void RenderVk::_resize() {
    LOG("resizing");
    _framebufferResized = false;
    vkDeviceWaitIdle(manager->deviceState.device);
    _initFrameResources();
}

void RenderVk::_startDraw() {
    if (!_frameResourcesCreated) {
      throw std::runtime_error("Tried to start draw when no"
                               " frame resources have been created"
                               " call LoadResourcesToGPU before "
			       "drawing to the screen");                               
    }    
    
    frameIndex = (frameIndex + 1) % MAX_CONCURRENT_FRAMES;
    checkResultAndThrow(frames[frameIndex]->waitForPreviousFrame(),
			"Render Error: failed to wait for previous frame fence");
    VkResult result = swapchain->acquireNextImage(
	    frames[frameIndex]->swapchainImageReady, &swapchainFrameIndex);
    if(result != VK_SUCCESS && !swapchainRecreationRequired(result))
	    checkResultAndThrow(result, "Render Error: failed to begin offscreen render pass!");
    checkResultAndThrow(frames[frameIndex]->startFrame(&currentCommandBuffer),
			"Render Error: Failed to start command buffer.");

    if(usingFinalRenderPass) 
	offscreenRenderPass->beginRenderPass(currentCommandBuffer, 0);
    else
	offscreenRenderPass->beginRenderPass(currentCommandBuffer, swapchainFrameIndex);

    for(auto pool: this->shaderPools)
	pool->setFrameIndex(frameIndex);
    
    currentBonesDynamicOffset = 0;
    currentModelPool = Resource::Pool();
    _begunDraw = true;
}	

void RenderVk::_store3DsetData() {
    VP3D->bindings[0].storeSetData(frameIndex, &VP3DData);
    VP3D->bindings[1].storeSetData(frameIndex, &timeData);
    lightingSet->setData(0, &lightingData);
}

void RenderVk::_store2DsetData() {
    VP2D->bindings[0].storeSetData(frameIndex, &VP2DData);
}

void RenderVk::_begin(RenderState state) {
    if (!_begunDraw)
	_startDraw();
    else if(_renderState == state)
	return;
    _drawBatch();
    _renderState = state;
    if(_current3DInstanceIndex == 0 && state != RenderState::Draw2D)
	_store3DsetData();
    if(_current2DInstanceIndex == 0 && state == RenderState::Draw2D)
	_store2DsetData();
    Pipeline* pipeline = nullptr;
    switch(state) {
    case RenderState::Draw2D:
	pipeline = &_pipeline2D;
	break;
    case RenderState::Draw3D:
	pipeline = &_pipeline3D;
	break;
    case RenderState::DrawAnim3D:
	pipeline = &_pipelineAnim3D;
	break;
    default:
	throw std::runtime_error("begin draw not implemented for this render state");
    }
    pipeline->begin(currentCommandBuffer, frameIndex);
}
  

  void RenderVk::DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMat) {
    if (_current3DInstanceIndex >= Resource::MAX_3D_BATCH) {
	LOG("WARNING: ran out of 3D instances!");
	return;
    }
    if(!_poolInUse(model.pool)) {
	LOG_ERROR("Tried Drawing with model in pool that is not in use");
	return;
    }
    
    _begin(RenderState::Draw3D);
    
    if (model != _currentModel)
	_drawBatch();
    
    _bindModelPool(model);
    _currentModel = model;
    perFrame3DData[_current3DInstanceIndex + _modelRuns].model = modelMatrix;
    perFrame3DData[_current3DInstanceIndex + _modelRuns].normalMat = normalMat;
    _modelRuns++;
    
    if (_current3DInstanceIndex + _modelRuns == Resource::MAX_3D_BATCH)
	_drawBatch();
}

void RenderVk::DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix,
			   glm::mat4 normalMat, Resource::ModelAnimation *animation) {
    if (_current3DInstanceIndex >= Resource::MAX_3D_BATCH) {
	LOG("WARNING: Ran out of 3D Anim Instance models!\n");
	return;
    }
    if(!_poolInUse(model.pool)) {
	LOG_ERROR("Tried Drawing with model in pool that is not in use");
	return;
    }
    _begin(RenderState::DrawAnim3D);
    if (model != _currentModel)
	_drawBatch();
    _bindModelPool(model);
    _currentModel = model;
    _currentColour = glm::vec4(0.0f);
    perFrame3DData[_current3DInstanceIndex + _modelRuns].model = modelMatrix;
    perFrame3DData[_current3DInstanceIndex + _modelRuns].normalMat = normalMat;
    _modelRuns++;

    auto animBones = animation->getCurrentBones();
    shaderStructs::Bones bonesData;
    for(int i = 0; i < animBones->size() && i < Resource::MAX_BONES; i++) {
	bonesData.mat[i] = animBones->at(i);
    }
    if(currentBonesDynamicOffset >= MAX_ANIMATIONS_PER_FRAME) {
	LOG("warning, too many animation calls!\n");
	return;
    }
    bones->bindings[0].storeSetData(frameIndex,
				    &bonesData, 0, 0, currentBonesDynamicOffset);
    uint32_t offset = static_cast<uint32_t>((currentBonesDynamicOffset) *
					    bones->bindings[0].bufferSize *
					    bones->bindings[0].setCount);
    _pipelineAnim3D.bindDynamicDS(currentCommandBuffer, &bones->set, frameIndex,  offset);
    _drawBatch();
    currentBonesDynamicOffset++;
}

void RenderVk::DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour, glm::vec4 texOffset) {
  if (_current2DInstanceIndex >= Resource::MAX_2D_BATCH) {
      LOG("WARNING: ran out of 2D instance models!\n");
      return;
  }
  if(!_poolInUse(texture.pool)) {
      LOG_ERROR("Tried Drawing with texture in pool that is not in use");
      return;
  }
  _begin(RenderState::Draw2D);
   perFrame2DVertData[_current2DInstanceIndex + _instance2Druns] = modelMatrix;
   perFrame2DFragData[_current2DInstanceIndex + _instance2Druns].colour = colour;
   perFrame2DFragData[_current2DInstanceIndex + _instance2Druns].texOffset = texOffset;
   perFrame2DFragData[_current2DInstanceIndex + _instance2Druns].texID =
       pools->get(texture.pool)->texLoader->getViewIndex(texture);
   
  _instance2Druns++;

  if (_current2DInstanceIndex + _instance2Druns == Resource::MAX_2D_BATCH)
    _drawBatch();
}

void RenderVk::DrawString(Resource::Font font, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour, float rotate) {
    auto draws = pools->get(font.pool)->fontLoader->DrawString(
	    font, text, position, size, depth, colour, rotate);
    for (const auto &draw : draws) {
	DrawQuad(draw.tex, draw.model, draw.colour, draw.texOffset);
    }
}

  void RenderVk::_bindModelPool(Resource::Model model) {
      if(currentModelPool.ID == Resource::NULL_POOL_ID || currentModelPool.ID != model.pool.ID) {
	  if(_modelRuns > 0)
	      _drawBatch();
	  if(!_poolInUse(model.pool))
	      throw std::runtime_error(
		      "Tried to bind model pool that is not in use");
	  pools->get(model.pool)->modelLoader->bindBuffers(currentCommandBuffer);
	  currentModelPool = model.pool;
      }
  }

void RenderVk::_drawBatch() {
    switch(_renderState) {
    case RenderState::DrawAnim3D:
    case RenderState::Draw3D:
	if(_current3DInstanceIndex + _modelRuns > Resource::MAX_3D_BATCH) {
	    if(_current3DInstanceIndex < Resource::MAX_3D_BATCH)
		_modelRuns = (Resource::MAX_3D_BATCH - _current3DInstanceIndex);
	    else
		_modelRuns = 0;
	    LOG("WARNING: Ran Out of 3D Instance Models");
	}
	if(_modelRuns == 0)
	    return;
	pools->get(currentModelPool)->modelLoader->drawModel(
		currentCommandBuffer,
		_pipeline3D.getLayout(),
		_currentModel,
		_modelRuns,
		_current3DInstanceIndex);
	_current3DInstanceIndex += _modelRuns;
	_modelRuns = 0;
	break;
    case RenderState::Draw2D:
	if(_current2DInstanceIndex + _instance2Druns > Resource::MAX_2D_BATCH) {
	    if(_current2DInstanceIndex < Resource::MAX_2D_BATCH)
		_instance2Druns = Resource::MAX_2D_BATCH - _current2DInstanceIndex;
	    else
		_instance2Druns = 0;
	    LOG("WARNING: Ran Out of 2D Instance Models");
	}
	if(_instance2Druns <= 0)
	    return;
	if(currentModelPool.ID == Resource::NULL_POOL_ID) {
	    pools->get(0)->modelLoader->bindBuffers(currentCommandBuffer);
	    currentModelPool = pools->get(0)->id();
	}
	pools->get(currentModelPool)->modelLoader->drawQuad(
		currentCommandBuffer,
		_pipeline2D.getLayout(),
		0, _instance2Druns,
		_current2DInstanceIndex,
		_currentColour,
		_currentTexOffset);
	_current2DInstanceIndex += _instance2Druns;
	_instance2Druns = 0;
	break;
    }
}


  VkSubmitInfo submitDrawInfo(Frame *frame, VkPipelineStageFlags *stageFlags) {
      VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = &frame->swapchainImageReady;
      submitInfo.pWaitDstStageMask = stageFlags;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &frame->commandBuffer;
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = &frame->drawFinished;
      return submitInfo;
  }

  VkPresentInfoKHR submitPresentInfo(VkSemaphore* waitSemaphore, VkSwapchainKHR *swapchain,
			     uint32_t* swapchainImageIndex) {
    VkPresentInfoKHR presentInfo {VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = waitSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapchain;
    presentInfo.pImageIndices = swapchainImageIndex;
    return presentInfo;
  }

void RenderVk::EndDraw(std::atomic<bool> &submit) {
  if (!_begunDraw)
    throw std::runtime_error("Tried to end draw before starting it");
  
  _begunDraw = false;
  _drawBatch();

  for (size_t i = 0; i < _current3DInstanceIndex; i++)
      perFrame3D->bindings[0].storeSetData(
	      frameIndex, &perFrame3DData[i], 0, i, 0);
  
  _current3DInstanceIndex = 0;

  for (size_t i = 0; i < _current2DInstanceIndex; i++) {
      perFrame2DVert->bindings[0].storeSetData(
	      frameIndex, &perFrame2DVertData[i], 0, i, 0);
  }
  perFrame2DSet->setData(0, perFrame2DFragData,
			 _current2DInstanceIndex * sizeof(shaderStructs::Frag2DData));
  
  _current2DInstanceIndex = 0;

  vkCmdEndRenderPass(currentCommandBuffer);

  // DO FINAL RENDER PASS

  if(usingFinalRenderPass) {
      finalRenderPass->beginRenderPass(currentCommandBuffer, swapchainFrameIndex);
  
      offscreenTransform->bindings[0].storeSetData(
	      frameIndex, &offscreenTransformData, 0, 0, 0);
      _pipelineFinal.begin(currentCommandBuffer, frameIndex);
      vkCmdDraw(currentCommandBuffer, 3, 1, 0, 0);
      
      vkCmdEndRenderPass(currentCommandBuffer);
  }
  
  VkResult result = vkEndCommandBuffer(currentCommandBuffer);
  if(result == VK_SUCCESS) {
      VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
      auto info = submitDrawInfo(frames[frameIndex], &stageFlags);
      VkResult result = vkhelper::submitQueue(
	      manager->deviceState.queue.graphicsPresentQueue,
	      &info, &graphicsPresentMutex, frames[frameIndex]->frameFinished);
      if(result != VK_SUCCESS)
	  LOG_ERR_TYPE("Render Error: Failed to sumbit draw commands.", result);
  }
  if(result == VK_SUCCESS) {
      VkSwapchainKHR sc = swapchain->getSwapchain();      
      auto info = submitPresentInfo(&frames[frameIndex]->drawFinished, &sc, &swapchainFrameIndex);
      graphicsPresentMutex.lock();
      VkResult result = vkQueuePresentKHR(manager->deviceState.queue.graphicsPresentQueue, &info);
      graphicsPresentMutex.unlock();
      if(result != VK_SUCCESS && !swapchainRecreationRequired(result))
	  LOG_ERR_TYPE("Render Error: Failed to sumbit draw commands.", result);
  }
  
  if (swapchainRecreationRequired(result) || _framebufferResized) {
      LOG("end of draw, resize or recreation required");
      _resize();
  } else if (result != VK_SUCCESS)
      checkResultAndThrow(result, "failed to present swapchain image to queue");
  
  submit = true;
}

//recreates frame resources, so any state change for rendering will be updated on next draw if this is called
void RenderVk::FramebufferResize() {
    _framebufferResized = true;
}


ShaderPool* RenderVk::CreateShaderPool() {
    shaderPools.push_back(
	    new ShaderPoolVk(manager->deviceState,
			     MAX_CONCURRENT_FRAMES));
    return shaderPools[shaderPools.size() - 1];
}

void RenderVk::DestroyShaderPool(ShaderPool* pool) {
    for(int i = 0; i < shaderPools.size(); i++)
	if(shaderPools[i] == pool) {
	    delete shaderPools[i];
	    shaderPools.erase(shaderPools.begin() + i);
	    return;
	}
    LOG_ERROR("Tried to destory shader pool but it was not "
	      "found in the list of created shader pools");
}

void RenderVk::LoadResourcesToGPU(ShaderPool* pool) {
    ((ShaderPoolVk*)pool)->CreateGpuResources(pools);
}
  

void RenderVk::set3DViewMat(glm::mat4 view, glm::vec4 camPos) {
    VP3DData.view = view;
    lightingData.camPos = camPos;
}

void RenderVk::set2DViewMat(glm::mat4 view) {
    VP2DData.view = view;
}

void RenderVk::set3DProjMat(glm::mat4 proj) {
    VP3DData.proj = proj;
    VP3DData.proj[1][1] *= -1; // glm has inverted y axis    
}

void RenderVk::set2DProjMat(glm::mat4 proj) {
    VP2DData.proj = proj;
    VP2DData.proj[1][1] *= -1;
    VP2DData.proj[3][1] *= -1;

    VP2DData.proj[2][2] *= -1;
    VP2DData.proj[3][2] *= -1;
}

void RenderVk::setLightingProps(BPLighting lighting) {
    lightingData = lighting;
}

}//namespace
