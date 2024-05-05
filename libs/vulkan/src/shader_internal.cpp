#include "shader_internal.h"

#include <cstring>
#include <stdexcept>

#include "parts/descriptors.h"
#include "parts/images.h"
#include "vkhelper.h"
#include "logger.h"


VkDescriptorType bindingTypeVk(Binding::type type);
VkShaderStageFlags shaderFlagsVk(shaderstage flags);


/// --- Shader Set ---


void SetVk::setData(size_t index,
		    void* data,
		    size_t bytesToRead,
		    size_t destinationOffset,
		    size_t arrayIndex,
		    size_t dynamicIndex) {
    try {
	InternalSet::setData(
		index, data, bytesToRead, destinationOffset,
		arrayIndex, dynamicIndex);
    } catch(std::invalid_argument e) {
	LOG_ERROR(e.what());
	return;
    }
    if(bytesToRead == 0)
	bytesToRead = bindings[index].typeSize - destinationOffset;
    bindings[index].setBuffer(
	    data, bytesToRead, destinationOffset, currentSetIndex, arrayIndex, dynamicIndex);
}

void SetVk::updateSampler(size_t index, size_t arrayIndex, TextureSampler sampler) {
    try{
	InternalSet::updateSampler(index, arrayIndex, sampler);
    } catch(std::invalid_argument &e) {
	LOG_ERROR(e.what());
	return;
    }
    if(!gpuResourcesCreated)
	return;
    samplersToDestroy.push_back(bindings[index].samplersVk[arrayIndex]);
    samplersTimeToLive = setHandles.size();
    bindings[index].createSampler(state, arrayIndex);
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<std::vector<VkDescriptorImageInfo>> imageVecs;
    bindings[index].writeSampler(arrayIndex, 1, writes, imageVecs, setHandles);
    LOG("Updating Sampler Descriptor");
    vkUpdateDescriptorSets(state.device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}

void SetVk::updateTextures(size_t index, size_t arrayIndex,
			  std::vector<Resource::Texture> textures) {
    try{
	InternalSet::updateTextures(index, arrayIndex, textures);
    } catch(std::invalid_argument &e) {
	LOG_ERROR(e.what());
	return;
    }
    if(!gpuResourcesCreated)
	return;
    bindings[index].getImageViews(arrayIndex, textures.size(), poolManager);
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<std::vector<VkDescriptorImageInfo>> imageVecs;
    LOG("Updating Texture Descriptor");
    bindings[index].writeTextures(arrayIndex, textures.size(), writes, imageVecs, setHandles);
    vkUpdateDescriptorSets(state.device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}


VkDescriptorSetLayout SetVk::CreateSetLayout() {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    for(uint32_t i = 0; i < bindings.size(); i++) {
	if(bindings[i].bindType == Binding::type::None)
	    continue;
	VkDescriptorSetLayoutBinding b;
	b.binding = i;
	bindings[i].index = i;
	b.descriptorCount = bindings[i].arrayCount;
	bindings[i].vkBindingType = bindingTypeVk(bindings[i].bindType);
	b.descriptorType = bindings[i].vkBindingType;
	bindings[i].vkStageFlags = shaderFlagsVk(this->stageFlags);
	b.stageFlags = bindings[i].vkStageFlags;
	b.pImmutableSamplers = VK_NULL_HANDLE;
	layoutBindings.push_back(b);

	VkDescriptorPoolSize p;
	p.descriptorCount = b.descriptorCount;
	p.type = b.descriptorType;
	poolSizes.push_back(p);
    }
    checkResultAndThrow(
	    part::create::DescriptorSetLayout(state.device, layoutBindings, &layout),
	    "Failed to create descriptor set layout");
    layoutCreated = true;
    return layout;
}

void SetVk::DestroySetResources() {
    if(layoutCreated) {
	vkDestroyDescriptorSetLayout(state.device, layout, nullptr);
	for(auto& binding: bindings)
	    binding.clear(state.device);
    }
    gpuResourcesCreated = false;
    layoutCreated = false;
}

VkFilter toVkFilter(TextureSampler::filter filter);
VkSamplerAddressMode toVKAddressMode(TextureSampler::address_mode addressMode);

void SetVk::setupDescriptorSets(
	size_t* pMemSize, VkPhysicalDeviceProperties deviceProps) {
    if(setHandles.size() <= 0)
	throw std::invalid_argument(
		"No descriptor sets were assinged to the set handles array");    
    for(auto &b: bindings) {
	switch(b.bindType) {
	case Binding::type::UniformBuffer:
	case Binding::type::UniformBufferDynamic:
	    b.calcBuffer(pMemSize,
			 deviceProps.limits.minUniformBufferOffsetAlignment,
			 setHandles.size());
	    break;
	case Binding::type::StorageBuffer:
	case Binding::type::StorageBufferDynamic:
	    b.calcBuffer(pMemSize,
			 deviceProps.limits.minStorageBufferOffsetAlignment,
			 setHandles.size());
	    break;
	case Binding::type::TextureSampler:
	    b.createSampler(state);
	    break;
	case Binding::type::Texture:
	    b.getImageViews(poolManager);
	default:
	    break;
	}
    }
}


void SetVk::writeDescriptorSets(void* p,
				VkBuffer buffer,
				std::vector<VkWriteDescriptorSet> &writes,
				std::vector<std::vector<VkDescriptorBufferInfo>> &buffers,
				std::vector<std::vector<VkDescriptorImageInfo>> &images) {
    for(auto &b: bindings){
	switch(b.bindType) {
	case Binding::type::UniformBuffer:
	case Binding::type::UniformBufferDynamic:
	case Binding::type::StorageBuffer:
	case Binding::type::StorageBufferDynamic:
	    b.writeBuffer(p, buffer, writes, buffers, setHandles);
	    break;
	case Binding::type::TextureSampler:
	    b.writeSampler(SIZE_MAX, 0, writes, images, setHandles);
	    break;
	case Binding::type::Texture:
	    b.writeTextures(SIZE_MAX, 0, writes, images, setHandles);
	    break;
	default:
	    continue;
	}
    }
    gpuResourcesCreated = true;
}

size_t SetVk::getDynamicOffset(size_t index, size_t dynamicIndex) {
    if(!dynamicBuffer(index))
	throw std::invalid_argument("Called getDynamicOffset on non dynamic buffer");
    if(dynamicIndex >= bindings[index].dynamicCount)
	throw std::invalid_argument("Called getDynamicOffset with out of range dynamic index");

    return bindings[index].dynamicMemSize * dynamicIndex;
}

bool SetVk::dynamicBuffer(size_t index) {
    if(index >= this->bindings.size())
	throw std::invalid_argument("Index to getDynamicOffset was greater than num bindings");
    return bindings[index].bindType == Binding::type::UniformBufferDynamic
	|| bindings[index].bindType == Binding::type::StorageBufferDynamic;
}

bool SetVk::dynamicBuffer() {
    for(int i = 0; i < bindings.size(); i++)
	if(dynamicBuffer(i))
	    return true;
    return false;
}

void SetVk::setHandleIndex(size_t handleIndex) {
    if(handleIndex >= setHandles.size())
	throw std::runtime_error("out of range descriptor set index");
    this->currentSetIndex = handleIndex;
    if(samplersToDestroy.size() > 0) {
	samplersTimeToLive--;
	if(samplersTimeToLive <= 0) {
	    for(auto &s: samplersToDestroy)
		vkDestroySampler(state.device, s, nullptr);
	    samplersToDestroy.clear();
	}		
    }
}


/// ---  Shader Pool ---


void ShaderPoolVk::CreateGpuResources() {
    InternalShaderPool::CreateGpuResources();
    createPool();
    createSets();
    createData();
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
	for(auto ps: set->getPoolSizes()) {
	    poolSizes.push_back(ps);
	    poolSizes.back().descriptorCount *= setCopies;
	}
    }
    checkResultAndThrow(part::create::DescriptorPool(
				state.device, &pool, poolSizes, sets.size() * setCopies),
			"Failed to create Descriptor Pool!");
}

void ShaderPoolVk::createSets() {
    std::vector<VkDescriptorSetLayout> setLayouts(sets.size() * setCopies);
    int layoutIndex = 0;
    for(auto &set: sets) {
	for(int i = 0; i < setCopies; i++)
	    setLayouts[layoutIndex++] = set->getLayout();
    }
    
    std::vector<VkDescriptorSet> setHandles(setLayouts.size());
    checkResultAndThrow(
	    part::create::DescriptorSets(state.device, pool, setLayouts, setHandles),
	    "Failed to create descriptor sets");
    
    int handleIndex = 0;
    for(auto set: sets) {
	std::vector<VkDescriptorSet> handles;
	for(int i = 0; i < setCopies; i++)
	    handles.push_back(setHandles[handleIndex++]);
	set->setDescSetHandles(handles);
    }
}

void ShaderPoolVk::createData() {
    VkPhysicalDeviceProperties deviceProps;
    vkGetPhysicalDeviceProperties(state.physicalDevice, &deviceProps);
    
    VkDeviceSize memorySize = 0;
    for(auto set: sets)
	set->setupDescriptorSets(&memorySize, deviceProps);

    // only support host coherent for now
    checkResultAndThrow(
	    vkhelper::createBufferAndMemory(
		    state, memorySize, &buffer, &memory,
		    VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),
	    "Failed to allocate memory and buffer for shader pool");
    
    vkBindBufferMemory(state.device, buffer, memory, 0);
    void *p;
    vkMapMemory(state.device, memory, 0, memorySize, 0, &p);
    
    std::vector<VkWriteDescriptorSet> writes;
    std::vector<std::vector<VkDescriptorBufferInfo>> buffers;
    std::vector<std::vector<VkDescriptorImageInfo>> images;
    for(auto set: sets)
    	set->writeDescriptorSets(p, buffer, writes, buffers, images);
    vkUpdateDescriptorSets(state.device, (uint32_t)writes.size(), writes.data(), 0, nullptr);
}



/// ----------- Bindings -----------


void BindingVk::clear(VkDevice device) {
    // buffers
    pData = nullptr;
    buffer = VK_NULL_HANDLE;
    baseOffset = 0;
    dataMemSize = 0;
    dynamicMemSize = 0;
    setMemSize = 0;
    // sampler
    if(this->bindType == Binding::type::TextureSampler) {
	for(auto &sampler: samplersVk)
	    vkDestroySampler(device, sampler, nullptr);
	samplersVk.clear();
    }
    textureViews.clear();
    textureLayouts.clear();
}

// ---- Binding Buffer ----

void BindingVk::calcBuffer(size_t* pMemSize, size_t alignment, size_t setCount) {
    // ------ memory layout ------------------------
    // | all sets data = | set1 | set 2 | ... |      |  num: set count (ie one per set handle)
    // | set = | dynamic data | dynamic data | ... | |  num: dynamic count
    // | dynamic data = | data | data | ... |        |  num: array count
    // ---------------------------------------------	
    *pMemSize = vkhelper::correctMemoryAlignment(*pMemSize, alignment);
    baseOffset = *pMemSize;
    dataMemSize = vkhelper::correctMemoryAlignment(typeSize, alignment);
    dynamicMemSize = dataMemSize * arrayCount;
    setMemSize = dynamicMemSize * dynamicCount;
    *pMemSize += setMemSize * setCount;
}

void BindingVk::writeBuffer(
	void *p, VkBuffer buff,
	std::vector<VkWriteDescriptorSet> &writes,
	std::vector<std::vector<VkDescriptorBufferInfo>> &buffers,
	std::vector<VkDescriptorSet> &sets) {
    this->pData = p;
    this->buffer = buff;
    size_t base = buffers.size();
    buffers.resize(base + sets.size());
    for(int setIndex = 0; setIndex < sets.size(); setIndex++) {
	// buff info points to the first dynamic data slot in memory
	//        <-point here->
	// set = | dynamic data | dynamic data | ... |
	std::vector<VkDescriptorBufferInfo>* buffs = &buffers[base + setIndex];
	for(int arrayIndex = 0; arrayIndex < arrayCount; arrayIndex++) {
	    VkDescriptorBufferInfo binfo;
	    binfo.buffer = buffer;
	    binfo.offset = baseOffset
		+ setMemSize * setIndex
		+ dataMemSize * arrayIndex;
	    binfo.range = dataMemSize;
	    buffs->push_back(binfo);
	}
	VkWriteDescriptorSet write = dsWrite(sets[setIndex]);
	write.pBufferInfo = buffs->data();
	writes.push_back(write);
    }
}

void BindingVk::setBuffer(void* data, size_t bytesToRead, size_t dstOffset,
			  size_t handleIndex, size_t arrayIndex, size_t dynamicIndex) {
    if(this->pData == nullptr)
	throw std::runtime_error("tried to write to buffer that has no memory assigned!");
    std::memcpy((unsigned char*)pData
		+ baseOffset
		+ handleIndex * setMemSize
		+ dynamicIndex * dynamicMemSize
		+ arrayIndex * dataMemSize
		+ dstOffset,
		data,
		bytesToRead);
}

/// --- Binding Sampler ----

void BindingVk::createSampler(DeviceState &state) {
    samplersVk.resize(samplerDescs.size());
    for(int i = 0; i < samplerDescs.size(); i++) {	
	createSampler(state, i);
    }
}

void BindingVk::createSampler(DeviceState &state, size_t index) {
    checkResultAndThrow(
	    part::create::TextureSampler(
		    state.device, state.physicalDevice,
		    &samplersVk[index],
		    samplerDescs[index].maxLod,
		    state.features.samplerAnisotropy,
		    toVkFilter(samplerDescs[index].textureFilter),
		    toVKAddressMode(samplerDescs[index].addressMode)),
	    "failed to create texture sampler!");    
}

void BindingVk::writeSampler(size_t updateIndex, size_t updateCount,
	std::vector<VkWriteDescriptorSet> &writes,
	std::vector<std::vector<VkDescriptorImageInfo>> &images,
	std::vector<VkDescriptorSet> &sets) {
    if(updateIndex == SIZE_MAX) {
	updateIndex = 0;
	updateCount = samplersVk.size();
    }
    if(updateCount == 0)
	return;
    size_t base = images.size();
    images.resize(base + sets.size());
    for(int i = 0; i < sets.size(); i++) {
	auto ims = &images[base + i];
	for(int arrayIndex = updateIndex; arrayIndex < updateIndex + updateCount; arrayIndex++) {
	    VkDescriptorImageInfo info;
	    info.sampler = samplersVk[arrayIndex];
	    ims->push_back(info);
	}
	VkWriteDescriptorSet write = dsWrite(updateIndex, updateCount, sets[i]);
	write.pImageInfo = ims->data();
	writes.push_back(write);
    }
}

/// ----- Binding Texture -----

void BindingVk::getImageViews(size_t updateIndex, size_t updateCount, PoolManagerVk *pools) {
    textureViews.resize(textures.size());
    textureLayouts.resize(textures.size());
    for(int i = updateIndex; i < updateCount && i < textures.size(); i++) {
	Resource::Texture t = textures[i];
	if(t.ID == Resource::NULL_ID) {
	    textureViews[i] = VK_NULL_HANDLE;
	    continue;
	}
	if(!pools->ValidPool(t.pool))
	    throw std::runtime_error("Shader Binding Vk: Passed pool was invalid");
	textureViews[i] = pools->get(t.pool)->texLoader->getImageView(t);
	textureLayouts[i] = pools->get(t.pool)->texLoader->getImageLayout(t);
    }
}

void BindingVk::getImageViews(PoolManagerVk *pools) {
    getImageViews(0, textures.size(), pools);
}

void BindingVk::writeTextures(size_t updateIndex, size_t updateCount,
			      std::vector<VkWriteDescriptorSet> &writes,
			      std::vector<std::vector<VkDescriptorImageInfo>> &images,
			      std::vector<VkDescriptorSet> &sets) {
    if(updateIndex == SIZE_MAX) {
	updateIndex = 0;
	updateCount = textureViews.size();
    }
    if(updateCount == 0)
	return;
    size_t base = images.size();
    images.resize(base + sets.size());
    for(int i = 0; i < sets.size(); i++) {
	auto ims = &images[base + i];
	for(int arrayIndex = updateIndex; arrayIndex < updateIndex + updateCount; arrayIndex++) {
	    if(textureViews[arrayIndex] == VK_NULL_HANDLE) {
		throw std::runtime_error(
			"TODO: skip null textures - need to update writes here properly");
	    }
	    VkDescriptorImageInfo info;
	    info.imageView = textureViews[arrayIndex];
	    info.imageLayout = textureLayouts[arrayIndex];
	    ims->push_back(info);
	}
	VkWriteDescriptorSet write = dsWrite(updateIndex, updateCount, sets[i]);
	write.pImageInfo = ims->data();
	writes.push_back(write);
    }
}


VkWriteDescriptorSet BindingVk::dsWrite(
	size_t updateIndex, size_t updateCount, VkDescriptorSet set) {
    VkWriteDescriptorSet write { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write.dstBinding = (uint32_t)index;
    write.dstArrayElement = (uint32_t)updateIndex;
    write.descriptorCount = (uint32_t)updateCount;
    write.descriptorType = vkBindingType;
    write.dstSet = set;
    return write;
}

VkWriteDescriptorSet BindingVk::dsWrite(VkDescriptorSet set) {
    return dsWrite(0, arrayCount, set);
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
	throw std::invalid_argument("Unrecognised shader set binding type, left as None?");
    }
}

VkShaderStageFlags shaderFlagsVk(shaderstage flags) {
    VkShaderStageFlags f = 0;

    if((unsigned int)flags & (unsigned int)shaderstage::vert)
	f |= VK_SHADER_STAGE_VERTEX_BIT;
    
    if((unsigned int)flags & (unsigned int)shaderstage::frag)
	f |= VK_SHADER_STAGE_FRAGMENT_BIT;
    
    return f;
}

VkFilter toVkFilter(TextureSampler::filter filter) {
    switch(filter) {
    case TextureSampler::filter::linear:
	return VK_FILTER_LINEAR;
    case TextureSampler::filter::nearest:
	return VK_FILTER_NEAREST;
    default:
	LOG_ERROR("Unrecognised filter for texture sampler");
	return VK_FILTER_LINEAR;
    }
}

VkSamplerAddressMode toVKAddressMode(TextureSampler::address_mode addressMode) {
    switch(addressMode) {
    case TextureSampler::address_mode::repeat:
	return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case TextureSampler::address_mode::mirrored_repeat:
	return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    case TextureSampler::address_mode::clamp_to_border:
	return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    case TextureSampler::address_mode::clamp_to_edge:
	return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    default:
	LOG_ERROR("Unrecognised address mode for texture sampler");
	return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    }
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
