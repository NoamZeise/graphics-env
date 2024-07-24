#include "pipeline.h"

#include "parts/pipeline.h"
#include "logger.h"

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


VkShaderModule createShaderModule(VkDevice device, std::vector<char>* shaderCode);

PipelineVk::PipelineVk(DeviceState& state,
		       Config config,
		       PipelineInput input,
		       std::vector<char> vertexShader,
		       std::vector<char> fragmentShader)
    : Pipeline(config, input, vertexShader, fragmentShader) {
    this->device = state.device;
    this->maxSamples = state.limits.maxMsaaSamples;
    vertexShaderModule = createShaderModule(device, &vertexShader);
    fragmentShaderModule = createShaderModule(device, &fragmentShader);
}

PipelineVk::~PipelineVk() {
    vkDestroyShaderModule(device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
    if(created)
	DestroyPipeline();
}


VkPipelineInputAssemblyStateCreateInfo inputAssembly();

VkPipelineVertexInputStateCreateInfo vertexInput(
	std::vector<VkVertexInputAttributeDescription>* vertexAttribDesc,
	std::vector<VkVertexInputBindingDescription>* vertexBindingDesc);

VkPipelineViewportStateCreateInfo viewportState(VkViewport* viewport, VkRect2D* scissor);

VkPipelineRasterizationStateCreateInfo rasterisationInfo(Pipeline::CullMode cullMode);

VkPipelineMultisampleStateCreateInfo multisampleInfo(Pipeline::Config config,
						     VkSampleCountFlagBits samples,
						     VkSampleCountFlagBits maxSamples);

VkPipelineDepthStencilStateCreateInfo depthStencilInfo(Pipeline::Config &config);

VkPipelineColorBlendAttachmentState blendAttachmentState(Pipeline::Config &config);

VkPipelineShaderStageCreateInfo shaderStageInfo(VkShaderModule module, VkShaderStageFlagBits stage);


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
    
    VkGraphicsPipelineCreateInfo pipelineInfo { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
    pipelineInfo.layout = layout;
    pipelineInfo.renderPass = rp->getRenderPass();

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = inputAssembly();
    pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;


    std::vector<VkVertexInputAttributeDescription> attribDesc = getAttribDesc(0, input);
    std::vector<VkVertexInputBindingDescription> bindDesc = {getBindingDesc(0, input)};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo = vertexInput(&attribDesc, &bindDesc);
    pipelineInfo.pVertexInputState = &vertexInputInfo;

    float minDepth = 0;
    float maxDepth = 0;
    VkExtent2D extent = rp->getExtent();
    VkViewport viewport{ 0.0f, 0.0f,
			 (float)extent.width, (float)extent.height,
			 minDepth, maxDepth };
    VkRect2D scissor{VkOffset2D{0, 0}, extent};
    VkPipelineViewportStateCreateInfo viewportInfo = viewportState(&viewport, &scissor);
    pipelineInfo.pViewportState = &viewportInfo;

    VkPipelineRasterizationStateCreateInfo rasterisationinfo = rasterisationInfo(config.cullMode);
    pipelineInfo.pRasterizationState = &rasterisationinfo;

    VkPipelineMultisampleStateCreateInfo multisampleinfo = multisampleInfo(
	    config, rp->msaaSamples(), maxSamples);
    pipelineInfo.pMultisampleState = &multisampleinfo;

    VkPipelineDepthStencilStateCreateInfo depthstencilinfo = depthStencilInfo(config);
    pipelineInfo.pDepthStencilState = &depthstencilinfo;

    VkPipelineColorBlendAttachmentState colourblendstate = blendAttachmentState(config);
    VkPipelineColorBlendStateCreateInfo colourblendinfo {
	VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    colourblendinfo.logicOpEnable = VK_FALSE;
    colourblendinfo.attachmentCount = 1;
    colourblendinfo.pAttachments = &colourblendstate;
    pipelineInfo.pColorBlendState = &colourblendinfo;

    VkPipelineDynamicStateCreateInfo dynamicStateInfo{
	VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamicStateInfo.dynamicStateCount = 0;
    dynamicStateInfo.pDynamicStates = nullptr;
    pipelineInfo.pDynamicState = &dynamicStateInfo;    
    
    VkPipelineShaderStageCreateInfo stages[2] = {
	shaderStageInfo(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT),
	shaderStageInfo(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT),
    };
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = stages;
    
    checkResultAndThrow(vkCreateGraphicsPipelines(
				device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline),
			"Failed to create graphics pipeline!");
}

void PipelineVk::DestroyPipeline() {
    vkDestroyPipeline(device, pipeline, nullptr);
    vkDestroyPipelineLayout(device, layout, nullptr);    
    Pipeline::DestroyPipeline();
}

/// ---- Helpers ----

VkShaderModule createShaderModule(VkDevice device, std::vector<char>* shaderCode) {
    VkShaderModule shader;
    
    VkShaderModuleCreateInfo info { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    info.codeSize = shaderCode->size();
    info.pCode = reinterpret_cast<const uint32_t*>(shaderCode->data());
    checkResultAndThrow(vkCreateShaderModule(device, &info, nullptr, &shader),
      			"Failed to compile shader");
    return shader;
}

// -- create pipeline helpers --


VkPipelineInputAssemblyStateCreateInfo inputAssembly() {
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo{
	VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
    return inputAssemblyInfo;
}

VkPipelineVertexInputStateCreateInfo vertexInput(
	std::vector<VkVertexInputAttributeDescription>* vertexAttribDesc,
	std::vector<VkVertexInputBindingDescription>* vertexBindingDesc) {
        // config vertex input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{
	VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexAttribDesc->size();
    vertexInputInfo.pVertexAttributeDescriptions = vertexAttribDesc->data();
    vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)vertexBindingDesc->size();
    vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc->data();
    return vertexInputInfo;
}

VkPipelineViewportStateCreateInfo viewportState(VkViewport* viewport, VkRect2D* scissor) {
    VkPipelineViewportStateCreateInfo viewportInfo{
	VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    viewportInfo.viewportCount = 1;
    viewportInfo.pViewports = viewport;
    viewportInfo.scissorCount = 1;
    viewportInfo.pScissors = scissor;
    return viewportInfo;
}

VkPipelineRasterizationStateCreateInfo rasterisationInfo(Pipeline::CullMode cullMode) {
    // config rasterization
    VkPipelineRasterizationStateCreateInfo rasterizationInfo{
	VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    rasterizationInfo.depthClampEnable = VK_FALSE;
    rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    switch(cullMode) {
    case Pipeline::CullMode::None:
	rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
	break;
    case Pipeline::CullMode::Back:
	rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	break;
    case Pipeline::CullMode::Front:
	rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	break;
    case Pipeline::CullMode::Both:
	rasterizationInfo.cullMode = VK_CULL_MODE_FRONT_AND_BACK;
	break;
    }
    rasterizationInfo.lineWidth = 1.0f;
    rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    return rasterizationInfo;
}

VkPipelineMultisampleStateCreateInfo multisampleInfo(Pipeline::Config config,
						     VkSampleCountFlagBits samples,
						     VkSampleCountFlagBits maxSamples) {
    VkPipelineMultisampleStateCreateInfo multisampleInfo{
	VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    
    if (samples != VK_SAMPLE_COUNT_1_BIT) {
	multisampleInfo.rasterizationSamples = samples;
	if (config.sampleShading) {
	    multisampleInfo.minSampleShading = 1.0f;
	    multisampleInfo.sampleShadingEnable = VK_TRUE;
	}
    } else {
	multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    }
    return multisampleInfo;
}

VkPipelineDepthStencilStateCreateInfo depthStencilInfo(Pipeline::Config &config) {
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{
	VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    if (config.depthTest) {
	depthStencilInfo.depthTestEnable = VK_TRUE;
	depthStencilInfo.depthWriteEnable = VK_TRUE;
    } else {
	depthStencilInfo.depthTestEnable = VK_FALSE;
	depthStencilInfo.depthWriteEnable = VK_FALSE;
    }
    
    depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    return depthStencilInfo;
}

VkBlendOp convertToVkFlags(Pipeline::BlendOp blendOp) {
    switch(blendOp) {
    case Pipeline::BlendOp::NoBlend:
	return VK_BLEND_OP_ADD;
    case Pipeline::BlendOp::Add:
	return VK_BLEND_OP_ADD;
    case Pipeline::BlendOp::Subtract:
	return VK_BLEND_OP_SUBTRACT;
    case Pipeline::BlendOp::ReverseSubtract:
	return VK_BLEND_OP_REVERSE_SUBTRACT;
    case Pipeline::BlendOp::Min:
	return VK_BLEND_OP_MIN;
    case Pipeline::BlendOp::Max:
	return VK_BLEND_OP_MAX;
    }
    throw std::runtime_error("Vulkan: Unrecognised blend op!");
}

VkPipelineColorBlendAttachmentState blendAttachmentState(Pipeline::Config &config) {
    VkPipelineColorBlendAttachmentState blendAttachment{};
    blendAttachment.colorWriteMask =
	  VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
	  VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (config.blendOp == Pipeline::BlendOp::NoBlend)
	  blendAttachment.blendEnable = VK_FALSE;
    else
	blendAttachment.blendEnable = VK_TRUE;
    blendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachment.colorBlendOp = convertToVkFlags(config.blendOp);
    blendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    blendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    blendAttachment.alphaBlendOp = blendAttachment.colorBlendOp;
    return blendAttachment;
}

VkPipelineShaderStageCreateInfo shaderStageInfo(
	VkShaderModule module, VkShaderStageFlagBits stage) {
    const char* SHADER_ENTRY_POINT = "main";
    VkPipelineShaderStageCreateInfo stageInfo {
	VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
    stageInfo.module = module;
    stageInfo.stage = stage;
    stageInfo.pName = SHADER_ENTRY_POINT;
    return stageInfo;
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
