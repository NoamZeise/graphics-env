#ifndef PIPELINE_H
#define PIPELINE_H

#include <volk.h>
#include <stdint.h>
#include <vector>

#include <graphics/pipeline.h>

#include "shader_buffers.h"



VkVertexInputBindingDescription getBindingDesc(uint32_t bindingIndex,
					       PipelineInput in);
VkVertexInputAttributeDescription getAttrib(uint32_t bindingIndex,
					    uint32_t location,
					    PipelineInput::Entry entry);
std::vector<VkVertexInputAttributeDescription> getAttribDesc(uint32_t bindingIndex,
							     PipelineInput in);

class PipelineOld {
public:
    PipelineOld() {};
    PipelineOld(VkPipelineLayout layout, VkPipeline pipeline,
	     std::vector<SetVk*> newSets);
    void begin(VkCommandBuffer cmdBuff, size_t frameIndex);
    void bindDynamicDSNew(
	    VkCommandBuffer cmdBuff, size_t frameIndex, size_t offsetIndex, size_t setIndex,
			  size_t bindingIndex);
    void destroy(VkDevice device);
    VkPipelineLayout getLayout() { return layout; }

private:
    VkPipelineLayout layout;
    VkPipeline pipeline;
    std::vector<SetVk*> newSets;
    std::vector<std::vector<uint32_t>> dynOffs;
};


#endif
