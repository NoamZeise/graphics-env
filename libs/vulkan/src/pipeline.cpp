#include "pipeline.h"

#include "parts/pipeline.h"

VkVertexInputBindingDescription getBindingDesc(uint32_t bindingIndex,
					       PipelineInput in) {
    VkVertexInputBindingDescription binding;
    binding.binding = bindingIndex;
    binding.stride = in.size;
    binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return binding;
}

VkVertexInputAttributeDescription getAttrib(uint32_t bindingIndex,
					    uint32_t location,
					    PipelineInput::Entry entry) {
    VkVertexInputAttributeDescription attrib;
    attrib.binding = bindingIndex;
    attrib.location = location;
    attrib.offset = entry.offset;
    switch(entry.input_type) {
    case PipelineInput::type::vec2:
	attrib.format = VK_FORMAT_R32G32_SFLOAT;
	break;
    case PipelineInput::type::vec3:
	attrib.format = VK_FORMAT_R32G32B32_SFLOAT;
	break;
    case PipelineInput::type::vec4:
	attrib.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	break;
    case PipelineInput::type::ivec4:
	attrib.format = VK_FORMAT_R32G32B32A32_SINT;
	break;
    }
    return attrib;
}

std::vector<VkVertexInputAttributeDescription> getAttribDesc(uint32_t bindingIndex,
							     PipelineInput in) {
    std::vector<VkVertexInputAttributeDescription> attribs(in.entries.size());
    for(int i = 0; i < attribs.size(); i++)
	attribs[i] = getAttrib(bindingIndex, i, in.entries[i]);
    return attribs;
}


/// ---- PipelineVk ----

void PipelineVk::CreatePipeline(void* renderpass) {
    Pipeline::CreatePipeline(renderpass);
    RenderPass* rp = (RenderPass*)renderpass;

    std::vector<VkPushConstantRange> pcs;
    for(auto& pc: pushConstants) {
	VkPushConstantRange range;
	range.size = pc.dataSize;
	range.offset = pc.offset;
	range.stageFlags = convertToVkFlags(pc.stageFlags);
	pcs.push_back(range);
    }

    std::vector<SetVk*> shaderSets(this->sets.size());
    for(int i = 0; i < this->sets.size(); i++) {
	shaderSets[i] = (SetVk*)this->sets[i];
    }

    this->layout = part::create::PipelineLayout(this->device, pcs, shaderSets);
}

void PipelineVk::DestroyPipeline() {
    vkDestroyPipelineLayout(device, layout, nullptr);
    Pipeline::DestroyPipeline();
}




/// ---- Pipeline Old ----


PipelineOld::PipelineOld(
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

void PipelineOld::begin(VkCommandBuffer cmdBuff, size_t frameIndex) {
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

void PipelineOld::bindDynamicDSNew(
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

void PipelineOld::destroy(VkDevice device) {
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, layout, nullptr);
}
