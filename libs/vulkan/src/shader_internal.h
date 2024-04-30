/// Lower level description of shader buffers for pipelines
/// holds the vulkan resources that make up the shader buffers.
///
/// Created using a higher level 'shader.h' description of the buffers.

#ifndef VKENV_SHADER_INTERNAL_H
#define VKENV_SHADER_INTERNAL_H

#include <volk.h>
#include <vector>
#include <cstring>

#include <render-internal/shader.h>

#include "shader.h"
#include "device_state.h"

struct BindingVk : public Binding {
    BindingVk() {}
    BindingVk(Binding binding) {
	Binding::operator=(binding);
    }

    void clear(VkDevice device);
    
    VkDescriptorType vkBindingType;
    VkShaderStageFlags vkStageFlags;

    /// Storage And Uniform Buffer Data
    
    VkBuffer buffer;
    // pointer to start of buffer for this binding
    // if it is host coherent (ie cpu visible)
    void* pData;
    
    // offset of this binding from start of bound memory
    // pData already has this added to it
    size_t baseOffset = 0;
    // size of base data with corrected alignment
    size_t dataMemSize = 0;
    /// Size of the data type * arrayCount
    size_t dynamicMemSize = 0;
    // Size of the binding for an individual set -> dynamicSize * dynamicCount
    size_t setMemSize = 0;

    VkSampler samplerVk;
};

class SetVk : public InternalSet {
public:
    SetVk(DeviceState state, stageflag flags) : InternalSet(flags) {
	this->state = state;
	DestroySetResources();
    }

    ~SetVk() override { DestroySetResources(); }    
    
    void setData(size_t index,
		 void* data,
		 size_t bytesToRead,
		 size_t destinationOffset,
		 size_t arrayIndex,
		 size_t dynamicIndex) override;
    
    // for temp pipeline changes
    VkDescriptorSetLayout getLayout() { return layout; }
    VkDescriptorSet* getSet(size_t index) {
	if(index >= setHandles.size())
	    throw std::runtime_error("out of range descriptor set index");
	return &setHandles[index];
    }

    void setHandleIndex(size_t handleIndex) {
	if(handleIndex >= setHandles.size())
	    throw std::runtime_error("out of range descriptor set index");
	this->handleIndex = handleIndex;
    }
    
    VkDescriptorSetLayout CreateSetLayout();    
    void DestroySetResources();
    std::vector<VkDescriptorPoolSize> getPoolSizes() { return poolSizes; }
    void setDescSetHandles(std::vector<VkDescriptorSet> handles) { this->setHandles = handles; }
    void getMemoryRequirements(size_t* pMemSize, VkPhysicalDeviceProperties deviceProps);
    void writeDescriptorSets(void* p,
			  VkBuffer buffer,
			  std::vector<VkWriteDescriptorSet> &writes,
			  std::vector<std::vector<VkDescriptorBufferInfo>> &buffers,
			  std::vector<std::vector<VkDescriptorImageInfo>> &images);

protected:
    
    Binding* getBinding(size_t index) override { return &bindings[index]; }
    size_t numBindings() override { return bindings.size(); }
    void setNumBindings(size_t size) override { bindings.resize(size); }
    void setBinding(size_t index, Binding binding) override {
	bindings[index] = BindingVk(binding);
    }

private:
    std::vector<BindingVk> bindings;
    
    DeviceState state;
    bool layoutCreated = false;
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorPoolSize> poolSizes;
    std::vector<VkDescriptorSet> setHandles;

    //for temp pipeline change
    size_t handleIndex;
};


class ShaderPoolVk : public InternalShaderPool {
public:
    ShaderPoolVk(DeviceState deviceState, int setCopies) {
	this->state = deviceState;
	this->setCopies = setCopies;
    }

    ~ShaderPoolVk() override {
	DestroyGpuResources();
	for(auto set: sets)
	    delete set;
    }
    
    Set* CreateSet(stageflag flags) override {
	sets.push_back(new SetVk(state, flags));
	return sets[sets.size() - 1];
    }

    void CreateGpuResources() override;
    void DestroyGpuResources() override;

    void setFrameIndex(size_t handleIndex) {
	for(auto set: sets)
	    set->setHandleIndex(handleIndex);
    }
    
private:

    void createPool();
    void createSets();
    void createData();
    
    DeviceState state;
    int setCopies;
    std::vector<SetVk*> sets;
    VkDescriptorPool pool;
    VkBuffer buffer;
    VkDeviceMemory memory;
};



/// -- OLD ---



namespace DS {
  struct DescriptorSet {
      void destroySet(VkDevice device);
      VkDescriptorSetLayout layout;
      std::vector<VkDescriptorSet> sets;
      std::vector<VkDescriptorPoolSize> poolSize;
      bool dynamicBuffer = false;
  };

  struct Binding {
      DescriptorSet *ds;
      VkDescriptorType type;

      size_t setCount;
      size_t dataTypeSize;
      size_t binding = 0;
      size_t descriptorCount = 1;
      size_t arraySize = 1;
      size_t dynamicBufferCount = 1;
      size_t bufferSize = 0;

      size_t offset;
      VkDeviceSize slotSize;
      void *pBuffer;
      VkImageView *imageViews;
      VkSampler *samplers;
      bool viewsPerSet = false;

      void storeSetData(size_t frameIndex, void *data, size_t descriptorIndex,
			size_t arrayIndex, size_t dynamicOffsetIndex);
      /// defaults other args to zero.
      /// ie for just a plain single struct descriptor
      void storeSetData(size_t frameIndex, void *data);

      void storeImageViews(VkDevice device);
  };
}

struct DescSet {
    DescSet(descriptor::Set set, size_t frameCount, VkDevice device);
    ~DescSet();
    descriptor::Set highLevelSet;
    DS::DescriptorSet set;
    std::vector<DS::Binding> bindings;
    VkDevice device;
};

#endif
