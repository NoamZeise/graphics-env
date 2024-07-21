#ifndef PARTS_RENDER_STYLE_H
#define PARTS_RENDER_STYLE_H

#include "../pipeline.h"
#include "../shader_buffers.h"
#include <volk.h>

#include <string>
#include <vector>

namespace part {
  namespace create {

    struct PipelineConfig {
	bool useDepthTest = true;
	bool useMultisampling;
	VkSampleCountFlagBits msaaSamples;
	bool useSampleShading;
	bool blendEnabled = true;
	VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT;
	VkBlendOp blendOp = VK_BLEND_OP_ADD;
    };

    VkPipelineLayout PipelineLayout(
	  VkDevice device,
	  std::vector<VkPushConstantRange> &pushConsts,
	  std::vector<SetVk*> &newSets);
    
    void GraphicsPipeline(VkDevice device,
			  PipelineOld* pipeline,
			  VkRenderPass renderPass,
			  std::vector<SetVk*> newSets,
			  std::vector<VkPushConstantRange> pushConstantsRanges,
			  std::string vertexShaderPath, std::string fragmentShaderPath,
			  VkExtent2D extent,
			  std::vector<VkVertexInputAttributeDescription> vertexAttribDesc,
			  std::vector<VkVertexInputBindingDescription> vertexBindingDesc,
			  PipelineConfig config);

  }
}

#endif
