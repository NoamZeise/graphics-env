#ifndef PARTS_VK_DESCRIPTORS_H
#define PARTS_VK_DESCRIPTORS_H

#include <volk.h>
#include <vector>

namespace part {
  namespace create {
    
    VkResult DescriptorSetLayout(VkDevice device,
				 std::vector<VkDescriptorSetLayoutBinding> &bindings,
				 VkDescriptorSetLayout *layout);
    
    VkResult DescriptorPool(VkDevice device,
			    VkDescriptorPool* pool,
			    std::vector<VkDescriptorPoolSize> &poolSizes,
			    uint32_t maxSets);

    VkResult DescriptorSets(VkDevice device,
			    VkDescriptorPool pool,
			    std::vector<VkDescriptorSetLayout> &layouts,
			    std::vector<VkDescriptorSet> &sets);      
  }
}

#endif
