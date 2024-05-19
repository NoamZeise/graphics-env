#ifndef OGL_SHADER_BUFFERS_INTERAL_H
#define OGL_SHADER_BUFFERS_INTERAL_H

#include <render-internal/shader_buffers.h>

struct BindingGl : public Binding {
    BindingGl() {}
    BindingGl(Binding binding) {
	Binding::operator=(binding);
    }
};

class SetGl : public InternalSet {
 public:
 SetGl(shader::Stage flags) : InternalSet(flags) {
	
    }
    ~SetGl() override { }

    void setData(size_t index,
		 void* data,
		 size_t bytesToRead,
		 size_t destinationOffset,
		 size_t arrayIndex,
		 size_t dynamicIndex,
		 bool setAllFrames) override {}
    
    void updateSampler(size_t index, size_t arrayIndex, TextureSampler sampler) override;

    void updateTextures(size_t index, size_t arrayIndex,
			std::vector<Resource::Texture> textures) override {}
    
 protected:

    Binding* getBinding(size_t index) override { return &bindings[index]; }
    size_t numBindings() override { return bindings.size(); }
    void setNumBindings(size_t size) override { bindings.resize(size); }
    void setBinding(size_t index, Binding binding) override {
	bindings[index] = Binding(binding);
    }
 private:
    std::vector<BindingGl> bindings;
};

class ShaderPoolGl : public InternalShaderPool {
 public:
    ShaderPoolGl() {}
    ~ShaderPoolGl() {
	for(SetGl* set: sets)
	    delete set;
    }

    ShaderSet* CreateSet(shader::Stage flags) override {
	sets.push_back(new SetGl(flags));
	return sets[sets.size() - 1];
    }

 private:
    std::vector<SetGl*> sets;
};


#endif /* OGL_SHADER_BUFFERS_INTERAL_H */
