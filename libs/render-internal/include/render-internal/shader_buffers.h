#ifndef GRAPHICS_INTERNAL_SHADER_BUFFERS_H
#define GRAPHICS_INTERNAL_SHADER_BUFFERS_H

#include <graphics/shader_buffers.h>
#include <string>

class InternalShaderPool : public ShaderPool {
public:
    virtual ~InternalShaderPool() {};
    void CreateGpuResources() override;
    void DestroyGpuResources() override;

private:
    bool resourcesCreated = false;
};

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
	    size_t dynamicCount);

    Binding(size_t arrayCount, std::vector<TextureSampler> samplers);

    Binding(size_t arrayCount, std::vector<Resource::Texture> textures);
    
    type bindType = type::None;
    size_t typeSize = 0;
    size_t arrayCount = 1;
    size_t dynamicCount = 1;
    std::vector<TextureSampler> samplerDescs;
    std::vector<Resource::Texture> textures;
};

class InternalSet : public Set {
 public:
    InternalSet(shaderstage stageFlags) {
	this->stageFlags = stageFlags;
    }

    virtual ~InternalSet() {}


    /// ----------- Shader Buffers -----------

    void addUniformBuffer(size_t index, size_t typeSize, size_t arrayCount,
			  size_t dynamicCount) override;

    void addStorageBuffer(size_t index, size_t typeSize, size_t arrayCount,
			  size_t dynamicCount) override;

    void setData(size_t index,
		 void* data,
		 size_t bytesToRead,
		 size_t destinationOffset,
		 size_t arrayIndex,
		 size_t dynamicIndex) override;


    /// ----------- Samplers -----------

    void addTextureSamplers(size_t index, size_t arrayCount,
			    std::vector<TextureSampler> samplers) override;
   
    void updateSampler(size_t index, size_t arrayIndex, TextureSampler sampler) override;
    
    TextureSampler getSampler(size_t index, size_t arrayIndex) override;


    /// ----------- Textures -----------

    void addTextures(size_t index, size_t arrayCount,
		     std::vector<Resource::Texture> textures) override;

    void updateTextures(size_t index, size_t arrayIndex,
			std::vector<Resource::Texture> textures) override;

    /// ----------- Helpers -----------

    size_t nextFreeIndex();

 protected:

    virtual Binding* getBinding(size_t index) = 0;    

    virtual size_t numBindings() = 0;    

    virtual void setNumBindings(size_t size) = 0;

    virtual void setBinding(size_t index, Binding binding) = 0;
    
    void addBinding(size_t index, Binding binding);
    
    shaderstage stageFlags;
    bool gpuResourcesCreated = false;
    
private:

    void throwOnBadIndexRange(size_t given, size_t max, std::string message);
    
    void checkSampler(size_t index, size_t arrayIndex);
};

#endif /* GRAPHICS_INTERNAL_SHADER_BUFFERS_H */
