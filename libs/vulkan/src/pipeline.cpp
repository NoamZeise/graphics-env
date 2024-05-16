#include "pipeline.h"

#include "logger.h"

Pipeline::Pipeline(
	VkPipelineLayout layout, VkPipeline pipeline,
	std::vector<SetVk*> newSets) {
    this->layout = layout;
    this->pipeline = pipeline;
    this->newSets = newSets;
    this->dynOffs.resize(newSets.size());
    for(int i = 0; i < newSets.size(); i++) {
	for(int j = 0; j < newSets[i]->bindingCount(); j++) {
	    if(newSets[i]->dynamicBuffer(j))
		dynOffs[i].push_back(0);
	}
    }
}

void Pipeline::begin(VkCommandBuffer cmdBuff, size_t frameIndex) {
    //bind non dynamic descriptor sets
    int bindOffset = 0;
    for (size_t i = 0; i < newSets.size(); i++) {
	vkCmdBindDescriptorSets(
		cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
		(uint32_t)i, 1,
		newSets[i]->getSet(frameIndex),
		dynOffs[i].size(), dynOffs[i].data()); // no dynamic yet
	bindOffset++;
    }
    
    vkCmdBindPipeline(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void Pipeline::bindDynamicDSNew(
	VkCommandBuffer cmdBuff, size_t frameIndex, size_t offsetIndex, size_t setIndex,
				size_t bindingIndex) {
    if(setIndex >= this->newSets.size())
	throw std::invalid_argument("bind new ds set index out of range");
    int newindex = (int)setIndex;
    if(newindex >= newSets.size() || newindex < 0)
	throw std::invalid_argument("bind new ds set index out of range");
    // assume first descriptor is dynamic for now, until pipeline rewrite
    dynOffs[newindex][bindingIndex]
	= newSets[newindex]->getDynamicOffset(bindingIndex, offsetIndex);
    vkCmdBindDescriptorSets(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
			    (uint32_t)(setIndex),
			    1, newSets[newindex]->getSet(frameIndex),
			    dynOffs[newindex].size(), dynOffs[newindex].data());
			    
}

void Pipeline::destroy(VkDevice device) {
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, layout, nullptr);
}
