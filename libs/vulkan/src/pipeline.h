#ifndef PIPELINE_H
#define PIPELINE_H

#include <volk.h>
#include <stdint.h>
#include <vector>

#include <graphics/pipeline.h>

#include "shader_buffers.h"

VkVertexInputBindingDescription getBindingDesc(uint32_t bindingIndex,
					       PipelineInput in);
std::vector<VkVertexInputAttributeDescription>
getAttribDesc(uint32_t bindingIndex, PipelineInput in);

class PipelineVk : public Pipeline {
public:
    PipelineVk(DeviceState& state,
	       Config config,
	       PipelineInput input,
	       std::vector<char> vertexShader,
	       std::vector<char> fragmentShader);
    ~PipelineVk();

    void CreatePipeline(void* renderpass) override;
    void DestroyPipeline() override;

    void BindShaderSet(VkCommandBuffer cmdBuff, size_t frameIndex);

    void BindPipeline(VkCommandBuffer cmdBuff);
private:
    VkDevice device;
    VkSampleCountFlagBits maxSamples;
    VkShaderModule vertexShaderModule;
    VkShaderModule fragmentShaderModule;
    
    VkPipelineLayout layout;
    VkPipeline pipeline;
};


/// ---- Old ----

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
