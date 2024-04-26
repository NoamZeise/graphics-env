/// Lower level description of shader buffers for pipelines
/// holds the vulkan resources that make up the shader buffers.
///
/// Created using a higher level 'shader.h' description of the buffers.

#ifndef VKENV_SHADER_INTERNAL_H
#define VKENV_SHADER_INTERNAL_H

#include <volk.h>
#include "shader.h"
#include <vector>

#include <render-internal/shader.h>

class SetVk : public InternalSet {
public:
    SetVk(VkDevice device, size_t setCopies, stageflag flags) : InternalSet(flags) {
	this->setCopies = setCopies;
	this->device = device;
    }

    ~SetVk() {
	if(layoutCreated)
	    vkDestroyDescriptorSetLayout(device, layout, nullptr);
    }
    
    void setData(size_t index, void* data) override {}

    void CreateSetLayout();
    std::vector<VkDescriptorPoolSize> getPoolSizes() { return poolSizes; }

private:
    size_t setCopies;
    VkDevice device;
    bool layoutCreated = false;
    VkDescriptorSetLayout layout;
    std::vector<VkDescriptorPoolSize> poolSizes;
};


class ShaderPoolVk : public InternalShaderPool {
public:
    ShaderPoolVk(VkDevice device, int setCopies) {
	this->device = device;
	this->setCopies = setCopies;
    }
    
    Set* CreateSet(stageflag flags) override {
	sets.push_back(SetVk(device, setCopies, flags));
	return &sets[sets.size() - 1];
    }

    void CreateGpuResources() override {
	std::vector<VkDescriptorPoolSize> poolSizes;
	for(auto &set: sets) {
	    set.CreateSetLayout();
	    for(auto& ps: set.getPoolSizes()) {
		poolSizes.push_back(ps);
	    }
	}
	
	InternalShaderPool::CreateGpuResources();
    }

    void DestroyGpuResources() override {
	sets.clear();
	InternalShaderPool::DestroyGpuResources();
    }
    
private:
    VkDevice device;
    int setCopies;
    std::vector<SetVk> sets;    
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
