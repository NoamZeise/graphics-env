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
    Binding() { bind_type = type::None; }

    Binding(type binding_type,
	    size_t typeSize,
	    size_t arrayCount,
	    size_t dynamicCount) {
	this->bind_type = binding_type;
	this->typeSize = typeSize;
	this->arrayCount = arrayCount;
	this->dynamicCount = dynamicCount;
    }
    
    type bind_type;
    size_t typeSize;
    size_t arrayCount;
    size_t dynamicCount;
};

class InternalSet : public Set {
 public:
    InternalSet(stageflag stageFlags) {
	this->stageFlags = stageFlags;
    }

    void addUniformBuffer(size_t index, size_t typeSize, size_t arrayCount) override {
	addBinding(index, Binding(Binding::type::UniformBuffer, typeSize, arrayCount, 1));
    }

    size_t nextFreeIndex() override {
	for(int i = 0; i < numBindings(); i++)
	    if(getBinding(i)->bind_type == Binding::type::None)
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
	if(getBinding(index)->bind_type != Binding::type::None)
	    throw std::runtime_error("Tried to add binding to Set for index already in use");
	setBinding(index, binding);
    };
    
    bool vaildSetData(size_t index, void* data) {
	if(index >= numBindings()) {
	    LOG_ERROR("Tried to set shader Set data to out of range index");
	    return false;
	}
	Binding* b = getBinding(index);
	if(b->bind_type == Binding::type::None ||
	   b->bind_type == Binding::type::Texture ||
	   b->bind_type == Binding::type::TextureSampler) {
	    LOG_ERROR("Tried to set shader Set data for texture or none element");
	    return false;
	}
	return true;
    }
    
    stageflag stageFlags;
};

class InternalShaderPool : public ShaderPool {
    
};


#endif
