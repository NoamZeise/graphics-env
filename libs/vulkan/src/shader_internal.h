/// Lower level description of shader buffers for pipelines
/// holds the vulkan resources that make up the shader buffers.
///
/// Created using a higher level 'shader.h' description of the buffers.

#ifndef VKENV_SHADER_INTERNAL_H
#define VKENV_SHADER_INTERNAL_H

#include <volk.h>
#include <vector>

#include <render-internal/shader.h>

#include "shader.h"
#include "device_state.h"

struct BindingVk : public Binding {
    BindingVk() {}
    BindingVk(Binding binding) :
	Binding(binding.bindType, binding.typeSize, binding.arrayCount, binding.dynamicCount) {}

    void clearMemoryOffsets();
    
    VkDescriptorType vkBindingType;
    VkShaderStageFlags vkStageFlags;

    /// Storage And Uniform Buffer Data
    
    VkBuffer buffer;
    void* pData; // for host coherent buffers
    
    // offset of this binding in descriptor pool memory
    size_t baseOffset = 0;
    // size of base data with corrected alignment
    size_t dataSize = 0;
    /// Size of the data type * no. dynamic copies
    size_t arrayElemSize = 0;
    // Size of the binding for an individual set -> array elem * array size
    size_t setSize = 0;
};

class SetVk : public InternalSet {
public:
    SetVk(VkDevice device, stageflag flags) : InternalSet(flags) {
	this->device = device;
	DestroySetResources();
    }

    ~SetVk() {
	DestroySetResources();
    }    
    
    void setData(size_t index, void* data) override {}

    VkDescriptorSetLayout CreateSetLayout();
    void DestroySetResources();
    std::vector<VkDescriptorPoolSize> getPoolSizes() { return poolSizes; }
    void setDescSetHandles(std::vector<VkDescriptorSet> handles) { this->setHandles = handles; }
    void getMemoryRequirements(size_t* pMemSize, VkPhysicalDeviceProperties deviceProps);
    void setMemoryPointer(void* p,
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
    
    VkDevice device;
    bool layoutCreated = false;
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorPoolSize> poolSizes;
    std::vector<VkDescriptorSet> setHandles;
};


class ShaderPoolVk : public InternalShaderPool {
public:
    ShaderPoolVk(DeviceState deviceState, int setCopies) {
	this->state = deviceState;
	this->setCopies = setCopies;
    }
    
    Set* CreateSet(stageflag flags) override {
	sets.push_back(SetVk(state.device, flags));
	return &sets[sets.size() - 1];
    }

    void CreateGpuResources() override;
    void DestroyGpuResources() override;
    
private:

    void createPool();
    void createSets();
    void createBuffer();
    
    DeviceState state;
    int setCopies;
    std::vector<SetVk> sets;
    VkDescriptorPool pool;
    VkBuffer buffer;
    VkDeviceMemory memory;
};




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
