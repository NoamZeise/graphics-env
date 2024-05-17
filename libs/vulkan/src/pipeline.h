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
#include "shader_buffers.h"

class Pipeline {
public:
    Pipeline() {};
    Pipeline(VkPipelineLayout layout, VkPipeline pipeline,
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
