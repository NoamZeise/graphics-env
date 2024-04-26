#ifndef GRAPHICS_ENV_RENDER_INTERNAL_SHADER_H
#define GRAPHICS_ENV_RENDER_INTERNAL_SHADER_H

#include <graphics/shader.h>
#include <graphics/logger.h>
#include <stdexcept>

// Pass to Set to create a binding
struct Binding {
public:
    enum class type {
	None,
	UniformBuffer,
	UniformBufferDynamic,
	StorageBuffer,
	StorageBufferDynamic,
	TextureSampler,
	Texture,
    };
    Binding() { binding_type = type::None; }

    Binding(type binding_type,
	    size_t typeSize,
	    size_t arrayCount,
	    size_t dynamicCount) {
	this->binding_type = binding_type;
	this->typeSize = typeSize;
	this->arrayCount = arrayCount;
	this->dynamicCount = dynamicCount;
    }   
    
    type binding_type;
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
	for(int i = 0; i < bindings.size(); i++)
	    if(bindings[i].binding_type == Binding::type::None)
		return i;
	return bindings.size();
    }

 protected:
    
    void addBinding(size_t index, Binding binding) {
	if(index + 1 > bindings.size()) {
	    bindings.resize(index + 1);
	}
	if(this->bindings[index].binding_type != Binding::type::None)
	    throw std::runtime_error("Tried to add binding to Set for index already in use"); 
	this->bindings[index] = binding;
    };

    bool missingBindings() {
	for(auto & b: bindings)
	    if(b.binding_type == Binding::type::None)
		return true;
	return false;
    }
    
    bool vaildSetData(size_t index, void* data) {
	if(index > bindings.size()) {
	    LOG_ERROR("Tried to set shader Set data to out of range index");
	    return false;
	}
	if(bindings[index].binding_type == Binding::type::None ||
	   bindings[index].binding_type == Binding::type::Texture ||
	   bindings[index].binding_type == Binding::type::TextureSampler) {
	    LOG_ERROR("Tried to set shader Set data for texture or none element");
	    return false;
	}
	return true;
    }
    
    stageflag stageFlags;
    std::vector<Binding> bindings;

};


#endif
