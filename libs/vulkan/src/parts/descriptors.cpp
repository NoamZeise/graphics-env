#include "descriptors.h"

namespace part {
  namespace create {
    
    VkResult DescriptorSetLayout(VkDevice device,
				 std::vector<VkDescriptorSetLayoutBinding> &bindings,
				 VkDescriptorSetLayout *layout) {
	VkDescriptorSetLayoutCreateInfo layoutInfo {
	    VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = (uint32_t)bindings.size();
	layoutInfo.pBindings = bindings.data();
	return vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, layout);
    }

    VkResult DescriptorPool(VkDevice device,
			    VkDescriptorPool* pool,
			    std::vector<VkDescriptorPoolSize> &poolSizes,
			    uint32_t maxSets) {
	VkDescriptorPoolCreateInfo poolInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
	poolInfo.poolSizeCount = (uint32_t)poolSizes.size();
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = maxSets;
	return vkCreateDescriptorPool(device, &poolInfo, nullptr, pool);
    }

    VkResult DescriptorSets(VkDevice device,
			    VkDescriptorPool pool,
			    std::vector<VkDescriptorSetLayout> &layouts,
			    std::vector<VkDescriptorSet> &sets) {
	VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = (uint32_t)layouts.size();
	allocInfo.pSetLayouts = layouts.data();
	return vkAllocateDescriptorSets(device, &allocInfo, sets.data());
    }
    
  } // namespace end
}
