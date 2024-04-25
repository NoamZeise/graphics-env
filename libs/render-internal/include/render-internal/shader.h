#ifndef GRAPHICS_ENV_RENDER_INTERNAL_SHADER_H
#define GRAPHICS_ENV_RENDER_INTERNAL_SHADER_H

#include <graphics/shader.h>

class InternalSet : public Set {
 public:
    InternalSet(stageflag stageFlags) {
	this->stageFlags = stageFlags;
    }

    void addBinding(size_t index, Binding binding) override {
	if(index + 1 > bindings.size()) {
	    bindings.resize(index + 1);
	}
	this->bindings[index] = binding;
    };

 protected:

    bool missingBindings() {
	for(auto & b: bindings)
	    if(b.binding_type == Binding::type::None)
		return true;
	return false;
    }
    
    stageflag stageFlags;
    std::vector<Binding> bindings;

};


#endif
