#include "shader_internal.h"

#include <cstring>
#include <stdexcept>

#include "parts/descriptors.h"
#include "vkhelper.h"


VkDescriptorType bindingTypeVk(Binding::type type);
VkShaderStageFlags shaderFlagsVk(stageflag flags);


/// --- Shader Set ---


VkDescriptorSetLayout SetVk::CreateSetLayout() {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;	
    for(uint32_t i = 0; i < bindings.size(); i++) {
	if(bindings[i].bindType == Binding::type::None)
	    continue;
	VkDescriptorSetLayoutBinding b;
	b.binding = i;
	b.descriptorCount = bindings[i].arrayCount;
	bindings[i].vkBindingType = bindingTypeVk(bindings[i].bindType);
	b.descriptorType = bindings[i].vkBindingType;
	bindings[i].vkStageFlags = shaderFlagsVk(this->stageFlags);
	b.stageFlags = bindings[i].vkStageFlags;
	layoutBindings.push_back(b);

	VkDescriptorPoolSize p;
	p.descriptorCount = b.descriptorCount;
	p.type = b.descriptorType;
	poolSizes.push_back(p);
    }
    part::create::DescriptorSetLayout(device, layoutBindings, &layout);
    layoutCreated = true;
    return layout;
}

void SetVk::DestroySetResources() {
    if(layoutCreated)
	vkDestroyDescriptorSetLayout(device, layout, nullptr);
    for(auto& binding: bindings)
	binding.clearMemoryOffsets();
    gpuResourcesCreated = false;
    layoutCreated = false;
}

void SetVk::getMemoryRequirements(size_t* pMemSize, VkPhysicalDeviceProperties deviceProps) {
    if(setHandles.size() <= 0)
	throw std::runtime_error(
		"No descriptor sets were assinged to the set handles array");
    
    for(auto &binding: bindings) {
	if(binding.bindType == Binding::type::None)
	    continue;
	
	// ------ memory layout ------------------------
	// | all sets data = | set1 | set 2 | ... |      |  num: set count (ie one per set handle)
	// | set = | dynamic data | dynamic data | ... | |  num: dynamic count
	// | dynamic data = | data | data | ... |        |  num: array count
	// ---------------------------------------------
	
	VkDeviceSize alignment;
	switch(binding.bindType) {
	case Binding::type::UniformBuffer:
	case Binding::type::UniformBufferDynamic:
	    alignment = deviceProps.limits.minUniformBufferOffsetAlignment;
	    break;
	case Binding::type::StorageBuffer:
	case Binding::type::StorageBufferDynamic:
	    alignment = deviceProps.limits.minStorageBufferOffsetAlignment;
	    break;
	default:
	    continue;
	}
	*pMemSize = vkhelper::correctMemoryAlignment(*pMemSize, alignment);
	binding.baseOffset = *pMemSize;
	binding.dataMemSize = vkhelper::correctMemoryAlignment(binding.typeSize, alignment);
	binding.dynamicMemSize = binding.dataMemSize * binding.arrayCount;
	binding.setMemSize = binding.dynamicMemSize * binding.dynamicCount;
	
	*pMemSize += binding.setMemSize * setHandles.size();
    }
}


void addBufferInfos(std::vector<VkWriteDescriptorSet> &writes,
		    std::vector<VkDescriptorBufferInfo> &buffInfos,
		    size_t setCount,
		    BindingVk* b,
		    VkBuffer buffer) {
    b->buffer = buffer;
    buffInfos.resize(setCount * b->arrayCount);
    for(int setIndex = 0; setIndex < setCount; setIndex++) {
	// buff info points to the first dynamic data slot in memory
	// then we offset into it when we bind the descriptor set
	//                    <----here---->
	// set memory = | dynamic data 1 memory | dynamic data 2 memory | ... |
	for(int arrayIndex = 0; arrayIndex < b->arrayCount; arrayIndex++) {
	    VkDescriptorBufferInfo binfo;
	    binfo.buffer = buffer;
	    binfo.offset = b->baseOffset
		+ b->setMemSize * setIndex
		+ b->dataMemSize * arrayIndex;
	    binfo.range = b->dataMemSize;
	    buffInfos[setIndex * b->arrayCount + arrayIndex] = binfo;
	}
	// assumes writes has >= setCount writes in vector
	writes[writes.size() - setCount + setIndex]
	    .pBufferInfo =  buffInfos.data() + setIndex * b->arrayCount;
    }
}

void SetVk::setMemoryPointer(void* p,
			     VkBuffer buffer,
			     std::vector<VkWriteDescriptorSet> &writes,
			     std::vector<std::vector<VkDescriptorBufferInfo>> &buffers,
			     std::vector<std::vector<VkDescriptorImageInfo>> &images) {
    for(int bindingIndex = 0; bindingIndex < bindings.size(); bindingIndex++) {
	BindingVk* b = &bindings[bindingIndex];
	if(b->bindType == Binding::type::None)
	    continue;
	
	for(int setIndex = 0; setIndex < setHandles.size(); setIndex++) {
	    VkWriteDescriptorSet write { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	    write.dstBinding = (uint32_t)bindingIndex;
	    write.dstArrayElement = 0; // updating whole array of bindings
	    write.descriptorCount = (uint32_t)b->arrayCount;
	    write.descriptorType = b->vkBindingType;
	    write.dstSet = setHandles[setIndex];
	    writes.push_back(write);
	}

	switch(b->bindType) {
	case Binding::type::UniformBuffer:
	case Binding::type::UniformBufferDynamic:
	case Binding::type::StorageBuffer:
	case Binding::type::StorageBufferDynamic:
	    b->pData = (unsigned char*)p + b->baseOffset;
	    buffers.push_back(std::vector<VkDescriptorBufferInfo>());
	    addBufferInfos(writes, buffers.back(), setHandles.size(), b, buffer);
	    break;
	case Binding::type::Texture:
	case Binding::type::TextureSampler:
	    throw std::runtime_error("image resources not implemented in sets");
	    break;
	default:
	    continue;
	}
    }
    gpuResourcesCreated = true;
}


/// ---  Shader Pool ---


void ShaderPoolVk::CreateGpuResources() {
    createPool();
    createSets();
    createBuffer();
    InternalShaderPool::CreateGpuResources();
}

void ShaderPoolVk::DestroyGpuResources() {
    for(auto &set: sets)
	set->DestroySetResources();
    vkDestroyBuffer(state.device, buffer, nullptr);
    vkFreeMemory(state.device, memory, nullptr);
    vkDestroyDescriptorPool(state.device, pool, nullptr); // also frees sets
    InternalShaderPool::DestroyGpuResources();
}

void ShaderPoolVk::createPool() {
    std::vector<VkDescriptorPoolSize> poolSizes;
    for(auto &set: sets) {
	set->CreateSetLayout();
	for(auto& ps: set->getPoolSizes()) {
	    poolSizes.push_back(ps);
	    poolSizes.back().descriptorCount *= setCopies;
	}
    }
    part::create::DescriptorPool(state.device, &pool, poolSizes, sets.size() * setCopies);
}

void ShaderPoolVk::createSets() {
    std::vector<VkDescriptorSetLayout> setLayouts(sets.size() * setCopies);
    int layoutIndex = 0;
    for(auto &set: sets) {
	for(int i = 0; i < setCopies; i++)
	    setLayouts[layoutIndex++] = set->getLayout();
    }
    
    std::vector<VkDescriptorSet> setHandles(setLayouts.size());
    part::create::DescriptorSets(state.device, pool, setLayouts, setHandles);
    
    int handleIndex = 0;
    for(auto set: sets) {
	std::vector<VkDescriptorSet> handles;
	for(int i = 0; i < setCopies; i++)
	    handles.push_back(setHandles[handleIndex++]);
	set->setDescSetHandles(handles);
    }
}

void ShaderPoolVk::createBuffer() {
    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(state.physicalDevice, &deviceProps);
    
    VkDeviceSize memorySize = 0;
    for(auto set: sets)
	set->getMemoryRequirements(&memorySize, deviceProps);

    // only support host coherent for now
    vkhelper::createBufferAndMemory(
	    state, memorySize, &buffer, &memory,
	    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
	    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    
    vkBindBufferMemory(state.device, buffer, memory, 0);
    void *p;
    vkMapMemory(state.device, memory, 0, memorySize, 0, &p);
    
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<std::vector<VkDescriptorBufferInfo>> buffers;
    std::vector<std::vector<VkDescriptorImageInfo>> images;
    for(auto set: sets)
    	set->setMemoryPointer(p, buffer, writes, buffers, images);
    vkUpdateDescriptorSets(state.device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

/// ---- Binding ----

void BindingVk::clearMemoryOffsets() {
    pData = nullptr;
    buffer = VK_NULL_HANDLE;
    baseOffset = 0;
    dataMemSize = 0;
    dynamicMemSize = 0;
    setMemSize = 0;
}



/// ----- Helpers -----


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
    
    if((unsigned int)flags & (unsigned int)stageflag::frag)
	f |= VK_SHADER_STAGE_FRAGMENT_BIT;
    
    return f;
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
