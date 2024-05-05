#ifndef PIPELINE_H
#define PIPELINE_H

#include <volk.h>
#include <GLFW/glfw3.h>

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>

#include <stdint.h>
#include <vector>
#include "shader_internal.h"

class Pipeline {
public:
    Pipeline() {};
    Pipeline(VkPipelineLayout layout, VkPipeline pipeline, std::vector<DS::DescriptorSet*> sets,
	     std::vector<SetVk*> newSets);
    void setDescSetState(DS::DescriptorSet* set, bool isActive);    
    void begin(VkCommandBuffer cmdBuff, size_t frameIndex);
    void bindDynamicDS(
	    VkCommandBuffer cmdBuff, DS::DescriptorSet *ds, size_t frameIndex, uint32_t dynOffset);
    void bindDynamicDSNew(
	    VkCommandBuffer cmdBuff, size_t frameIndex, size_t offsetIndex, size_t setIndex,
			  size_t bindingIndex);
    void destroy(VkDevice device);
    VkPipelineLayout getLayout() { return layout; }

private:
    VkPipelineLayout layout;
    VkPipeline pipeline;
    std::vector<DS::DescriptorSet*> descriptorSets;
    std::vector<bool> descriptorSetsActive;
    std::vector<SetVk*> newSets;
    std::vector<std::vector<uint32_t>> dynOffs;
};


#endif
