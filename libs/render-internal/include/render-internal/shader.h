#ifndef GRAPHICS_ENV_RENDER_INTERNAL_SHADER_H
#define GRAPHICS_ENV_RENDER_INTERNAL_SHADER_H

#include <graphics/shader.h>
#include <graphics/logger.h>
#include <stdexcept>

// Pass to Set to create a binding
struct Binding {
    enum class type {
	None,
	UniformBuffer,
	UniformBufferDynamic,
	StorageBuffer,
	StorageBufferDynamic,
	TextureSampler,
	Texture,
    };
    
    Binding() {}

    Binding(type binding_type,
	    size_t typeSize,
	    size_t arrayCount,
	    size_t dynamicCount) {
	this->bindType = binding_type;
	this->typeSize = typeSize;
	this->arrayCount = arrayCount;
	this->dynamicCount = dynamicCount;
    }

    Binding(size_t arrayCount, std::vector<TextureSampler> samplers) {
	this->bindType = type::TextureSampler;
	this->samplerDescs = samplers;
	this->arrayCount = arrayCount;
    }

    Binding(size_t arrayCount, std::vector<Resource::Texture> textures) {
	this->bindType = type::Texture;
	this->textures = textures;
	this->arrayCount = arrayCount;
    }
    
    type bindType = type::None;
    size_t typeSize = 0;
    size_t arrayCount = 1;
    size_t dynamicCount = 1;
    std::vector<TextureSampler> samplerDescs;
    std::vector<Resource::Texture> textures;
};

class InternalSet : public Set {
 public:
    InternalSet(stageflag stageFlags) {
	this->stageFlags = stageFlags;
    }

    virtual ~InternalSet() {}


    /// ----------- Shader Buffers -----------

    void addUniformBuffer(size_t index, size_t typeSize, size_t arrayCount) override {
	if(arrayCount == 0 || typeSize == 0)
	    throw std::invalid_argument("array count or type size of uniform buffer equaled 0");
	    
	addBinding(index, Binding(Binding::type::UniformBuffer, typeSize, arrayCount, 1));
    }

    void addStorageBuffer(size_t index, size_t typeSize, size_t arrayCount) override {
	if(arrayCount == 0 || typeSize == 0)
	    throw std::invalid_argument("array count or type size of uniform buffer equaled 0");
	    
	addBinding(index, Binding(Binding::type::StorageBuffer, typeSize, arrayCount, 1));
    }

    void setData(
	    size_t index,
	    void* data,
	    size_t bytesToRead,
	    size_t destinationOffset,
	    size_t arrayIndex,
	    size_t dynamicIndex) override {
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


    /// ----------- Samplers -----------

    void addTextureSamplers(size_t index, size_t arrayCount, std::vector<TextureSampler> samplers) override {
	if(arrayCount < samplers.size()) {
	    LOG_ERROR("Set::addTextureSamplers : Passed a vector of samplers "
		      "With more elements than arrayCount. Truncating to fit arrayCount");
	    samplers.resize(arrayCount);
	}
	addBinding(index, Binding(arrayCount, samplers));
    }
   
    void updateSampler(size_t index, size_t arrayIndex, TextureSampler sampler) override {
	throwOnBadIndexRange(index, numBindings(), "binding index");
	if(getBinding(index)->bindType != Binding::type::TextureSampler &&
	   getBinding(index)->bindType != Binding::type::None)
	    throw std::invalid_argument(
		    "Update Sampler Error: Tried to update sampler for non sampler index");
	if(arrayIndex >= getBinding(index)->arrayCount)
	    throw std::invalid_argument(
		    "Update Sampler Error: Sampler array index greater than sampler array size. "
		    " Given index " + std::to_string(arrayIndex) +
		    " But the array size of the descriptor was " +
		    std::to_string(getBinding(index)->arrayCount));
	
	if(arrayIndex >= getBinding(index)->samplerDescs.size()) {
	    getBinding(index)->samplerDescs.resize(arrayIndex + 1);
	}
	getBinding(index)->samplerDescs[arrayIndex] = sampler;
    }


    /// ----------- Textures -----------

    void addTextures(size_t index, size_t arrayCount,
		     std::vector<Resource::Texture> textures) override {
	addBinding(index, Binding(arrayCount, textures));
    }

    size_t nextFreeIndex() override {
	for(int i = 0; i < numBindings(); i++)
	    if(getBinding(i)->bindType == Binding::type::None)
		return i;
	return numBindings();
    }

 protected:

    virtual Binding* getBinding(size_t index) = 0;    
    virtual size_t numBindings() = 0;    
    virtual void setNumBindings(size_t size) = 0;
    virtual void setBinding(size_t index, Binding binding) = 0;
    
    void addBinding(size_t index, Binding binding) {
	if(index >= numBindings()) {
	    setNumBindings(index + 1);
	}
	if(getBinding(index)->bindType != Binding::type::None)
	    throw std::invalid_argument("Tried to add binding to Set for index already in use");
	setBinding(index, binding);
    };
    
    stageflag stageFlags;
    bool gpuResourcesCreated = false;
    
private:

    void throwOnBadIndexRange(size_t given, size_t max, std::string message) {
	if(given >= max)
	    throw std::invalid_argument(
		    "Shader Set Error - in update data: "
		    + message + " out of range - given index: " + std::to_string(given)
		    + "   max index: " + std::to_string(max));
    }    
};

class InternalShaderPool : public ShaderPool {
public:
    virtual ~InternalShaderPool() {};
    virtual void CreateGpuResources() {
	if(resourcesCreated)
	    DestroyGpuResources();
	resourcesCreated = true;
    }
    void DestroyGpuResources() override {
	resourcesCreated = false;
    }

private:
    bool resourcesCreated = false;
};


#endif
