#include "model_loader.h"

#include <stdexcept>
#include <cstring>

#include "../vkhelper.h"
#include "../logger.h"
#include "../pipeline_data.h"
#include "../parts/threading.h"

struct GPUMeshVk : public GPUMesh {
    GPUMeshVk() {}

    GPUMeshVk(MeshData* mesh, uint32_t indexOffset, uint32_t vertexOffset) : GPUMesh(mesh) {
	this->indexCount = mesh->indices.size();
	this->indexOffset = indexOffset;
	this->vertexOffset = vertexOffset;
    }
    uint32_t indexCount = 0;
    uint32_t indexOffset = 0;
    uint32_t vertexOffset = 0;
};

struct GPUModelVk : public GPUModel {
    std::vector<GPUMeshVk> meshes;
    uint32_t vertexCount = 0;
    uint32_t indexCount  = 0;
    // unused as there my be multiple vertex data types
    // in use in a model loader, so we rebind the
    // vertex buffer each time a model is drawn.
    // -> in future:
    // Could optimize by packining same vertex types
    // together in buffer, then using vertex offset
    // and only rebinding vertex buffer when the type changes.
    // or checking if only one type of vertex is used by all models
    // then using only a single vertex buffer binding
    uint32_t vertexOffset = 0;
    uint32_t indexOffset = 0;
    uint32_t vertexDataOffset = 0;

    GPUModelVk(ModelData *model) : GPUModel(model) {}
    
    void draw(VkCommandBuffer cmdBuff,
	      uint32_t meshIndex,
	      uint32_t instanceCount,
	      uint32_t instanceOffset) {
	if(meshIndex >= meshes.size()) {
	    LOG_ERROR("Mesh Index out of range. "
		      " - index: " << meshIndex <<
		      " - mesh count: " << meshes.size());
	    return;
	}
	vkCmdDrawIndexed(
		cmdBuff,
		meshes[meshIndex].indexCount,
		instanceCount,
		meshes[meshIndex].indexOffset
		+ indexOffset,
		meshes[meshIndex].vertexOffset
		+ vertexOffset,
		instanceOffset);
    }
};
	
ModelLoaderVk::ModelLoaderVk(DeviceState base, VkCommandPool cmdpool,
			     VkCommandBuffer generalCmdBuff,
			     Resource::Pool pool, BasePoolManager* pools)
    : InternalModelLoader(pool, pools){
      this->base = base;
      this->cmdpool = cmdpool;
      this->cmdbuff = generalCmdBuff;
      checkResultAndThrow(part::create::Fence(base.device, &loadedFence, false),
			  "failed to create finish load semaphore in model loader");
}

ModelLoaderVk::~ModelLoaderVk() {
    vkDestroyFence(base.device, loadedFence, nullptr);
    clearGPU();
}

void ModelLoaderVk::clearGPU() {
    vertexDataSize = 0;
    indexDataSize = 0;
    if(models.empty())
	return;
    for(GPUModelVk* model: models)
	delete model;
    models.clear();      
      
    vkDestroyBuffer(base.device, buffer, nullptr);
    vkFreeMemory(base.device, memory, nullptr);
}

void ModelLoaderVk::bindBuffers(VkCommandBuffer cmdBuff) {
    vkCmdBindIndexBuffer(cmdBuff, buffer, vertexDataSize, VK_INDEX_TYPE_UINT32);
}

GPUModelVk* ModelLoaderVk::getModel(VkCommandBuffer cmdBuff, Resource::Model model) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in draw with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return nullptr;
    }

    GPUModelVk *modelInfo = models[model.ID];

    VkBuffer vertexBuffers[] = { buffer };
    VkDeviceSize offsets[] = { modelInfo->vertexDataOffset };
    vkCmdBindVertexBuffers(cmdBuff, 0, 1, vertexBuffers, offsets);
    return modelInfo;
}

void ModelLoaderVk::drawModel(VkCommandBuffer cmdBuff,
			      VkPipelineLayout layout,
			      Resource::Model model,
			      uint32_t count,
			      uint32_t instanceOffset) {
    if(count == 0)
	return;

    GPUModelVk* modelInfo = getModel(cmdBuff, model);
    if(modelInfo == nullptr) return;
    
    for(size_t i = 0; i < modelInfo->meshes.size(); i++) {
	fragPushConstants fps {
	    model.colour.a == 0.0f ? modelInfo->meshes[i].diffuseColour : model.colour,
	    glm::vec4(0, 0, 1, 1),
	    modelGetTexID(model, modelInfo->meshes[i].texture, pools),
	};
	vkCmdPushConstants(cmdBuff, layout, VK_SHADER_STAGE_FRAGMENT_BIT,
			   0, sizeof(fragPushConstants), &fps);
	modelInfo->draw(cmdBuff, (uint32_t)i, count, instanceOffset);
    }
}

void ModelLoaderVk::drawQuad(VkCommandBuffer cmdBuff,
			     VkPipelineLayout layout,
			     uint32_t count,
			     uint32_t instanceOffset) {
    if(count == 0)
	return;
    GPUModelVk* modelInfo = getModel(cmdBuff, quad);
    if(modelInfo == nullptr) return;
    modelInfo->draw(cmdBuff, 0, count, instanceOffset);    
}

void ModelLoaderVk::loadGPU() {
    clearGPU();

    loadQuad();

    processModelData();

    LOG("Loading model data - size: " << vertexDataSize + indexDataSize);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingMemory;

    if(vkhelper::createBufferAndMemory(
	       base, vertexDataSize + indexDataSize, &stagingBuffer, &stagingMemory,
	       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
       != VK_SUCCESS) {
	throw std::runtime_error("Failed to create staging buffer for model data");
    }

    vkBindBufferMemory(base.device, stagingBuffer, stagingMemory, 0);
    void* pMem;
    vkMapMemory(base.device, stagingMemory, 0, vertexDataSize + indexDataSize, 0, &pMem);

    stageModelData(pMem);
    clearStaged();

    LOG("Copying Model Data to GPU");

    vkhelper::createBufferAndMemory(base, vertexDataSize + indexDataSize, &buffer, &memory,
				    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
				    VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
				    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkBindBufferMemory(base.device, buffer, memory, 0);


    VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(cmdbuff, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = vertexDataSize + indexDataSize;
    vkCmdCopyBuffer(cmdbuff, stagingBuffer, buffer, 1, &copyRegion);
    vkEndCommandBuffer(cmdbuff);

    checkResultAndThrow(vkhelper::submitCmdBuffAndWait(
				base.device,
				base.queue.graphicsPresentQueue,
				&cmdbuff, loadedFence,
				&graphicsPresentMutex),
			"failed to submit model load commands");

    //free staging buffer
    vkDestroyBuffer(base.device, stagingBuffer, nullptr);
    vkFreeMemory(base.device, stagingMemory, nullptr);

    LOG("finished loading model data to gpu");
}

void ModelLoaderVk::processModelData() {
    uint32_t modelVertexOffset = 0;
    models.resize(staged.size());
    for(int i = 0; i < staged.size(); i++) {
	GPUModelVk* model = new GPUModelVk(staged[i]);
	// zero for now as may be multiple vertex types packed together
	model->vertexOffset = 0;//modelVertexOffset;
	model->vertexDataOffset = vertexDataSize;
	model->indexOffset = indexDataSize / sizeof(staged[i]->meshes[0]->indices[0]);
	model->meshes.resize(staged[i]->meshes.size());
	for(int j = 0 ; j <  staged[i]->meshes.size(); j++) {
	    MeshData* mesh = staged[i]->meshes[j];
	    model->meshes[j] = GPUMeshVk(mesh,
					model->indexCount,  //as offset
					model->vertexCount //as offset
					);
	    model->vertexCount += (uint32_t)mesh->vertexCount;
	    model->indexCount  += (uint32_t)mesh->indices.size();
	    vertexDataSize += (uint32_t)(
		    model->vertType.size * mesh->vertexCount);
	    indexDataSize +=  (uint32_t)(
		    sizeof(mesh->indices[0]) * mesh->indices.size());
	}
	modelVertexOffset += model->vertexCount;
	
	models[i] = model;
    }
}

void ModelLoaderVk::stageModelData(void* pMem) {
    size_t vertexOffset = 0;
    size_t indexOffset = vertexDataSize;
    for(auto model: staged) {
	for(size_t i = 0; i < model->meshes.size(); i++) {
	      
	    std::memcpy(static_cast<char*>(pMem) + vertexOffset,
			model->meshes[i]->vertices,
			model->format.size * model->meshes[i]->vertexCount);
	      
	    vertexOffset += model->format.size * model->meshes[i]->vertexCount;
	      
	    std::memcpy(static_cast<char*>(pMem) + indexOffset,
			model->meshes[i]->indices.data(),
			sizeof(model->meshes[i]->indices[0])
			* model->meshes[i]->indices.size());
	      
	    indexOffset += sizeof(model->meshes[i]->indices[0])
		* model->meshes[i]->indices.size();	      
	}
    }    
}

Resource::ModelAnimation ModelLoaderVk::getAnimation(Resource::Model model, std::string animation) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in getAnimation with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return Resource::ModelAnimation();
    }
    return models[model.ID]->getAnimation(animation);
}

Resource::ModelAnimation ModelLoaderVk::getAnimation(Resource::Model model, int index) {
    if(model.ID >= models.size()) {
	LOG_ERROR("in getAnimation with out of range model. id: "
                  << model.ID << " -  model count: " << models.size());
	return Resource::ModelAnimation();
    }
    return models[model.ID]->getAnimation(index);
}
