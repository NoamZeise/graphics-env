#include "shader_internal.h"

#include <cstring>
#include <stdexcept>

#include "parts/descriptors.h"

#include <stdexcept>

VkDescriptorType bindingTypeVk(Binding::type type) {
    switch (type) {
    case Binding::type::UniformBuffer:
	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    case Binding::type::UniformBufferDynamic:
	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    case Binding::type::StorageBuffer:
	return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    case Binding::type::StorageBufferDynamic:
	return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
    case Binding::type::Texture:
	return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    case Binding::type::TextureSampler:
	return VK_DESCRIPTOR_TYPE_SAMPLER;
    default:
	throw std::runtime_error("Unrecognised shader set binding type, left as None?");
    }
}

VkShaderStageFlags shaderFlagsVk(stageflag flags) {
    VkShaderStageFlags f = 0;
    if((unsigned int)flags & (unsigned int)stageflag::vert)
	f |= VK_SHADER_STAGE_VERTEX_BIT;
    if((unsigned int)flags & (unsigned int)stageflag::vert)
	f |= VK_SHADER_STAGE_FRAGMENT_BIT;
    return f;
}

void SetVk::CreateSetLayout() {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;	
    for(uint32_t i = 0; i < bindings.size(); i++) {
	if(bindings[i].binding_type == Binding::type::None)
	    continue;
	VkDescriptorSetLayoutBinding b;
	b.binding = i;
	b.descriptorCount = bindings[i].arrayCount;
	b.descriptorType = bindingTypeVk(bindings[i].binding_type);
	b.stageFlags = shaderFlagsVk(this->stageFlags);
	layoutBindings.push_back(b);

	VkDescriptorPoolSize p;
	p.descriptorCount = b.descriptorCount;
	p.type = b.descriptorType;
	poolSizes.push_back(p);
    }
    part::create::DescriptorSetLayout(device, layoutBindings, &layout);
    layoutCreated = true;
}








DescSet::DescSet(descriptor::Set set, size_t frameCount, VkDevice device) {
  this->highLevelSet = set;
  this->device = device;

  for(auto& desc: set.descriptors) {
    DS::Binding binding;
    binding.ds = &(this->set);
    if(desc.isSingleArrayStruct) {
      binding.arraySize = desc.dataArraySize;
    } else {
      binding.descriptorCount = desc.dataArraySize;
    }
    binding.setCount = frameCount;
    binding.imageViews = nullptr;
    binding.samplers = nullptr;
    binding.dataTypeSize = desc.dataTypeSize;
    switch(desc.type) {
    case descriptor::Type::UniformBuffer:
      binding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
      break;
    case descriptor::Type::UniformBufferDynamic:
      this->set.dynamicBuffer = true;
      binding.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
      break;
    case descriptor::Type::StorageBuffer:
      binding.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
      break;
    case descriptor::Type::StorageBufferDynamic:
      this->set.dynamicBuffer = true;
      binding.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
      break;
    case descriptor::Type::Sampler:
      binding.type = VK_DESCRIPTOR_TYPE_SAMPLER;
      binding.samplers = (VkSampler*)desc.pSamplerOrImageViews;
      break;
    case descriptor::Type::SampledImage:
      binding.viewsPerSet = desc.differentViewPerSet;
      binding.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
      binding.imageViews = (VkImageView*)desc.pSamplerOrImageViews;
      break;
    default:
      throw std::runtime_error("Unrecognised descriptor type in DescSet contructor");
    }
    binding.dynamicBufferCount = desc.dynamicBufferSize;
    this->bindings.push_back(binding);
  }
  VkShaderStageFlagBits stage;
  switch(set.shaderStage) {
  case descriptor::ShaderStage::Vertex:
      stage = VK_SHADER_STAGE_VERTEX_BIT;
      break;
  case descriptor::ShaderStage::Fragment:
      stage = VK_SHADER_STAGE_FRAGMENT_BIT;
      break;
  default:
      throw std::runtime_error("Unrecognised shader stage in DescSet constructor");
  }
  //until ds are moved all to new system, just pass ref as before
  std::vector<DS::Binding*> bindingRef(bindings.size());
  for(int i = 0; i < bindings.size(); i++) {
      bindingRef[i] = &bindings[i];
  }
  part::create::DescriptorSetLayout(device, &this->set, bindingRef, stage);
}


DescSet::~DescSet() { set.destroySet(device); }

namespace DS {
  void DescriptorSet::destroySet(VkDevice device) {
      vkDestroyDescriptorSetLayout(device, layout, nullptr);
  }
  void Binding::storeSetData(size_t frameIndex, void *data, size_t descriptorIndex,
			     size_t arrayIndex, size_t dynamicOffsetIndex) {
      if (type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER ||
	  type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER ||
	  type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC ||
	  type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
	  std::memcpy(
		  static_cast<char *>(pBuffer) + offset + dynamicOffsetIndex*setCount*bufferSize +
		  ((frameIndex * bufferSize) + (descriptorIndex * arraySize * slotSize) +
		   (arrayIndex * slotSize)),
		  data, dataTypeSize);
      else
	  throw std::runtime_error("Descriptor Shader Buffer: tried to store data "
				   "in non uniform or storage buffer!");
  }

  ///TODO ADD THESE FOR ALL TYPES + MAKE PREPARE SHADER BUFFER
  // USE THESE INSTEAD
  void Binding::storeImageViews(VkDevice device) {
      if(type != VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE)
	  throw std::runtime_error("Descriptor Shader Buffer: tried to store image views "
				   "in non sampled-image binding!");
      std::vector<VkWriteDescriptorSet> writes(
	      setCount,
	      {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET});
      std::vector<VkDescriptorImageInfo> imageInfo(setCount * descriptorCount);
      for(int i = 0; i < setCount; i++) {
	  //images
	  for(int j = 0; j < descriptorCount; j++) {
	      size_t imageIndex = (descriptorCount * i) + j;
	      imageInfo[imageIndex].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	      if(viewsPerSet)
		  imageInfo[imageIndex].imageView = *(imageViews +
						      (i * descriptorCount)
						      + j);
	      else
		  imageInfo[imageIndex].imageView = *(imageViews + j);
	  }
	  writes[i].dstSet = ds->sets[i];
	  writes[i].dstBinding = binding;
	  writes[i].dstArrayElement = 0;
	  writes[i].descriptorCount = (uint32_t)descriptorCount;
	  writes[i].descriptorType = type;
	  writes[i].pImageInfo = imageInfo.data() + (i * descriptorCount);
	  
      }
      vkUpdateDescriptorSets(device, writes.size(), writes.data(), 0, nullptr);
  }

  void Binding::storeSetData(size_t frameIndex, void *data) {
      storeSetData(frameIndex, data, 0, 0, 0);
  }
}
