#include <render-internal/shader_buffers.h>

#include <stdexcept>
#include <graphics/logger.h>


/// ----------- Internal Shader Pool -----------

void InternalShaderPool::CreateGpuResources() {
    if(resourcesCreated)
	DestroyGpuResources();
    resourcesCreated = true;
}
void InternalShaderPool::DestroyGpuResources() {
    resourcesCreated = false;
}


/// ----------- Bindings -----------

Binding::Binding(type binding_type,
		 size_t typeSize,
		 size_t arrayCount,
		 size_t dynamicCount) {
    this->typeSize = typeSize;
    this->arrayCount = arrayCount;
    this->dynamicCount = dynamicCount;	
    this->bindType = binding_type;
    if(dynamicCount > 1) {
	if(this->bindType == type::UniformBuffer)
	    this->bindType = type::UniformBufferDynamic;
	if(this->bindType == type::StorageBuffer)
	    this->bindType = type::StorageBufferDynamic;
    }
}

Binding::Binding(size_t arrayCount, std::vector<TextureSampler> samplers) {
    this->bindType = type::TextureSampler;
    this->samplerDescs = samplers;
    this->arrayCount = arrayCount;
}

Binding::Binding(size_t arrayCount, std::vector<Resource::Texture> textures) {
    this->bindType = type::Texture;
    this->textures = textures;
    this->arrayCount = arrayCount;
}

std::string Binding::to_string(type t) {
    switch(t) {
    case type::None:
	return "None";
    case type::StorageBuffer:
	return "StorageBuffer";
    case type::StorageBufferDynamic:
	return "StorageBufferDynamic";
    case type::UniformBuffer:
	return "UniformBuffer";
    case type::UniformBufferDynamic:
	return "UniformBufferDynamic";
    case type::Texture:
	return "Texture";
    case type::TextureSampler:
	return "TextureSampler";
    default:
	throw std::runtime_error(
		"Unrecognised binding type in internal render");
    }
}

/// ----------- Internal Set -----------

InternalSet::InternalSet(shader::Stage stageFlags) {    
    this->stageFlags = stageFlags;
    if(!this->stageFlags) {
	throw std::invalid_argument(
		"Shader Set given no shader stage flags");
    }
}

/// ----------- Shader Buffers -----------

void InternalSet::addUniformBuffer(size_t index, size_t typeSize, size_t arrayCount,
				   size_t dynamicCount)  {
    if(arrayCount == 0 || typeSize == 0 || dynamicCount == 0)
	throw std::invalid_argument("array count or type size or dynamic count "
				    "of uniform buffer equaled 0");
    addBinding(
	    index,
	    Binding(Binding::type::UniformBuffer, typeSize, arrayCount, dynamicCount));
}

void InternalSet::addStorageBuffer(size_t index, size_t typeSize, size_t arrayCount,
				   size_t dynamicCount)  {
    if(arrayCount == 0 || typeSize == 0 || dynamicCount == 0)
	throw std::invalid_argument("array count or type size or dynamic count "
				    "of uniform buffer equaled 0");	
    addBinding(
	    index,
	    Binding(Binding::type::StorageBuffer, typeSize, arrayCount, dynamicCount));
}


void InternalSet::setData(size_t index,
			  void* data,
			  size_t bytesToRead,
			  size_t destinationOffset,
			  size_t arrayIndex,
			  size_t dynamicIndex,
			  bool setAllFrames)  {
    if(!gpuResourcesCreated)
	throw std::invalid_argument(
		"Shader Set Error:  Tried to setData for set "
		"Whose parent pool has not created Gpu Resources for it");
    throwOnBadIndexRange(index, numBindings(), "binding index");	
    Binding* b = getBinding(index);
    if(b->bindType != Binding::type::UniformBuffer &&
       b->bindType != Binding::type::UniformBufferDynamic &&
       b->bindType != Binding::type::StorageBuffer &&
       b->bindType != Binding::type::StorageBufferDynamic) {
	throw std::invalid_argument(
		"Shader Set Error: Tried to setData for non buffer element");
    }
    throwOnBadIndexRange(arrayIndex, b->arrayCount, "array index");
    throwOnBadIndexRange(arrayIndex, b->dynamicCount, "dynamic index");
    if(bytesToRead == 0)
	bytesToRead = b->typeSize - destinationOffset;
    throwOnBadIndexRange(bytesToRead, b->typeSize - destinationOffset + 1,
			 "tried to set too much memory "
			 "(bytes to read >= type size * destination offset),");
}


/// ----------- Shader Set Samplers -----------

void InternalSet::addTextureSamplers(size_t index, size_t arrayCount, std::vector<TextureSampler> samplers)  {
    if(arrayCount < samplers.size()) {
	LOG_ERROR("Set::addTextureSamplers : Passed a vector of samplers "
		  "With more elements than arrayCount. Truncating to fit arrayCount");
	samplers.resize(arrayCount);
    }
    addBinding(index, Binding(arrayCount, samplers));
}
   
void InternalSet::updateSampler(size_t index, size_t arrayIndex, TextureSampler sampler)  {	
    checkSampler(index, arrayIndex);
    if(arrayIndex >= getBinding(index)->samplerDescs.size()) {
	getBinding(index)->samplerDescs.resize(arrayIndex + 1);
    }
    getBinding(index)->samplerDescs[arrayIndex] = sampler;
}
    
TextureSampler InternalSet::getSampler(size_t index, size_t arrayIndex) {
    checkSampler(index, arrayIndex);
    return getBinding(index)->samplerDescs[arrayIndex];
}


/// ----------- Shader Set Textures -----------

void InternalSet::addTextures(size_t index, size_t arrayCount,
			      std::vector<Resource::Texture> textures) {
    addBinding(index, Binding(arrayCount, textures));
}

void InternalSet::updateTextures(size_t index, size_t arrayIndex,
				 std::vector<Resource::Texture> textures) {
    throwOnBadIndexRange(index, numBindings(), "binding index");
    if(getBinding(index)->bindType != Binding::type::Texture)
	throw std::invalid_argument(
		"Update Texture Error: Tried to update texture for non texture index");
    throwOnBadIndexRange(arrayIndex,
			 getBinding(index)->arrayCount, "arrayIndex out of range");
    throwOnBadIndexRange(arrayIndex + textures.size(),
			 getBinding(index)->arrayCount + 1, "too many textures");
	
    if(arrayIndex + textures.size() > getBinding(index)->textures.size())
	getBinding(index)->textures.resize(arrayIndex + textures.size());
    for(int i = 0; i < textures.size(); i++)
	getBinding(index)->textures[arrayIndex + i] = textures[i];
}


/// ----------- Shader Set Helpers -----------

size_t InternalSet::nextFreeIndex() {
    for(int i = 0; i < numBindings(); i++)
	if(getBinding(i)->bindType == Binding::type::None)
	    return i;
    return numBindings();
}

void InternalSet::addBinding(size_t index, Binding binding) {
    if(index >= numBindings()) {
	setNumBindings(index + 1);
    }
    if(getBinding(index)->bindType != Binding::type::None)
	throw std::invalid_argument("Tried to add binding to Set for index already in use");
    setBinding(index, binding);
};


void InternalSet::throwOnBadIndexRange(size_t given, size_t max, std::string message) {
    if(given >= max)
	throw std::invalid_argument(
		"Shader Set Error - in update data: "
		+ message + " out of range - given index: " + std::to_string(given)
		+ "   max index: " + std::to_string(max));
}

void InternalSet::checkSampler(size_t index, size_t arrayIndex) {
    throwOnBadIndexRange(index, numBindings(), "binding index");
    if(getBinding(index)->bindType != Binding::type::TextureSampler)
	throw std::invalid_argument(
		"Update Sampler Error: Tried to update sampler for non sampler index");
    throwOnBadIndexRange(arrayIndex, getBinding(index)->arrayCount, "array index");
}
