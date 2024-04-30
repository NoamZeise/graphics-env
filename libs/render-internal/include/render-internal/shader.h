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
    
    Binding() { bindType = type::None; }

    Binding(type binding_type,
	    size_t typeSize,
	    size_t arrayCount,
	    size_t dynamicCount) {
	this->bindType = binding_type;
	this->typeSize = typeSize;
	this->arrayCount = arrayCount;
	this->dynamicCount = dynamicCount;
    }
    
    type bindType;
    size_t typeSize;
    size_t arrayCount;
    size_t dynamicCount;
};

class InternalSet : public Set {
 public:
    InternalSet(stageflag stageFlags) {
	this->stageFlags = stageFlags;
    }

    virtual ~InternalSet() {}

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

    void setData(size_t index, void* data) override {
	setData(index, data, 0, 0, 0, 0);
    }

    void setData(size_t index, void* data, size_t size) override {
	setData(index, data, size, 0, 0, 0);
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
	if(b->bindType == Binding::type::None ||
	   b->bindType == Binding::type::Texture ||
	   b->bindType == Binding::type::TextureSampler) {
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
		    "Shader Set Error - in setData: "
		    + message + " out of range - given index: " + std::to_string(given)
		    + "   max index: " + std::to_string(max));
    }    
};

class InternalShaderPool : public ShaderPool {
public:
    virtual ~InternalShaderPool() {};
    void CreateGpuResources() override {
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
