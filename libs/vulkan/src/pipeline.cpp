#include "pipeline.h"

#include "logger.h"

Pipeline::Pipeline(
	VkPipelineLayout layout, VkPipeline pipeline, std::vector<DS::DescriptorSet*> sets,
	std::vector<SetVk*> newSets) {
    this->descriptorSets = sets;
    this->layout = layout;
    this->pipeline = pipeline;
    this->descriptorSetsActive = std::vector<bool>(descriptorSets.size(), true);
    this->newSets = newSets;
    this->dynOffs.resize(newSets.size());
    for(int i = 0; i < newSets.size(); i++) {
	for(int j = 0; j < newSets[i]->bindingCount(); j++) {
	    if(newSets[i]->dynamicBuffer(j))
		dynOffs[i].push_back(0);
	}
    }
}

void Pipeline::setDescSetState(DS::DescriptorSet *set, bool isActive) {
    for(int i = 0; i < descriptorSets.size(); i++) {
	if(descriptorSets[i] == set) {
	    descriptorSetsActive[i] = isActive;
	}
    }
}

void Pipeline::begin(VkCommandBuffer cmdBuff, size_t frameIndex) {
    //bind non dynamic descriptor sets
    int bindOffset = 0;
    for (size_t i = 0; i < descriptorSets.size(); i++) {
	if(descriptorSetsActive[i] &&
	   !descriptorSets[i]->dynamicBuffer &&
	   descriptorSets[i]->sets.size() != 0)
	    vkCmdBindDescriptorSets(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
				    static_cast<uint32_t>(i - bindOffset), 1,
				    &descriptorSets[i]->sets[frameIndex],
				    0, nullptr);
	else if(!descriptorSetsActive[i]) {
	    LOG("inactive slot: " << i);
	    bindOffset++;
	}
    }
    
    for (size_t i = 0; i < newSets.size(); i++) {
	vkCmdBindDescriptorSets(
		cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
		(uint32_t)(descriptorSets.size() + i), 1,
		newSets[i]->getSet(frameIndex),
		dynOffs[i].size(), dynOffs[i].data()); // no dynamic yet
	bindOffset++;
    }
    
    vkCmdBindPipeline(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

void Pipeline::bindDynamicDS(
	VkCommandBuffer cmdBuff, DS::DescriptorSet *ds, size_t frameIndex, uint32_t dynOffset) {
    for (size_t i = 0; i < descriptorSets.size(); i++)
	if(descriptorSets[i]->dynamicBuffer)
	    if(descriptorSets[i] == ds)
		vkCmdBindDescriptorSets(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
					static_cast<uint32_t>(i), 1,
					&descriptorSets[i]->sets[frameIndex],
					1, &dynOffset);
}

void Pipeline::bindDynamicDSNew(
	VkCommandBuffer cmdBuff, size_t frameIndex, size_t offsetIndex, size_t setIndex,
				size_t bindingIndex) {
    if(setIndex >= this->newSets.size() + this->descriptorSets.size())
	throw std::invalid_argument("bind new ds set index out of range");
    int newindex = (int)setIndex - (int)this->descriptorSets.size();
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
