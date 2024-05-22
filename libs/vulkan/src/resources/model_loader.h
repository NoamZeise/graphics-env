#ifndef MODEL_RENDER_H
#define MODEL_RENDER_H

#include <resource-loaders/model_loader.h>
#include "../device_state.h"

struct ModelInGPU;

class ModelLoaderVk : public InternalModelLoader {
public:
    ModelLoaderVk(DeviceState base, VkCommandPool cmdpool, VkCommandBuffer generalCmdBuff,
		  Resource::Pool pool, BasePoolManager *pools);
    ~ModelLoaderVk() override;
    void loadGPU() override;
    void clearGPU() override;

    void bindBuffers(VkCommandBuffer cmdBuff);

    void drawModel(VkCommandBuffer cmdBuff, VkPipelineLayout layout, Resource::Model model,
		   uint32_t count, uint32_t instanceOffset);
    
    void drawQuad(VkCommandBuffer cmdBuff,
		  VkPipelineLayout layout,
		  uint32_t count,
		  uint32_t instanceOffset);
    
    Resource::ModelAnimation getAnimation(Resource::Model model,
					  std::string animationName) override;
    Resource::ModelAnimation getAnimation(Resource::Model model,
					  int index) override;

private:
    
    void processModelData();

    void stageModelData(void* pMem);

    ModelInGPU* getModel(VkCommandBuffer cmdBuff, Resource::Model model);
    
    void drawMesh(VkCommandBuffer cmdBuff,
		  ModelInGPU *modelInfo,
		  uint32_t meshIndex,
		  uint32_t instanceCount,
		  uint32_t instanceOffset);

    DeviceState base;
    VkCommandPool cmdpool;
    VkCommandBuffer cmdbuff;
    VkFence loadedFence;
    std::vector<ModelInGPU*> models;
    VkBuffer buffer;
    VkDeviceMemory memory;

    uint32_t vertexDataSize = 0;
    uint32_t indexDataSize = 0;
};



#endif
