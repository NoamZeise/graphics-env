#ifndef GRAPHICS_API_SHADER_BUFFERS_H
#define GRAPHICS_API_SHADER_BUFFERS_H

#include "resources.h"
#include "shader_info.h"
#include <vector>

class ShaderSet;

/// Holds shader sets
class ShaderPool {
public:
    /// shader stage flags - this specifies which shader stages
    ///                      the set should work in
    virtual ShaderSet* CreateSet(shader::Stage shaderStageFlags) = 0;
    // automatically destroys created resources if they were already created
    virtual void CreateGpuResources() = 0;
    virtual void DestroyGpuResources() = 0;
};

class ShaderSet {
public:    
    /// ------ Shader Buffers -------    
    /// index -> binding index in shader set
    /// typeSize -> size of data structure you want to store
    /// arrayCount -> how many of that data structure you store in the shader
    /// dynamicCount -> how many times you want to be able to swap the data per frame
    
    virtual void addUniformBuffer(size_t index, size_t typeSize, size_t arrayCount,
				  size_t dynamicCount) = 0;
    
            void addUniformBuffer(size_t index, size_t typeSize, size_t arrayCount) {
		addUniformBuffer(index, typeSize, arrayCount, 1);}
    
            void addUniformBuffer(size_t index, size_t typeSize) {
		addUniformBuffer(index, typeSize, 1); }
    
    virtual void addStorageBuffer(size_t index, size_t typeSize, size_t arrayCount,
				  size_t dynamicCount) = 0;
    
            void addStorageBuffer(size_t index, size_t typeSize, size_t arrayCount) {
		addStorageBuffer(index, typeSize, arrayCount, 1); }
    
            void addStorageBuffer(size_t index, size_t typeSize) {
		addStorageBuffer(index, typeSize, 1); }
    
    /// Set the data of a shader buffer per frame
    void setData(size_t index, void* data) {
	setData(index, data, 0, 0, 0, 0, false);
    }
    void setData(size_t index, void* data, size_t bytesToRead) {
	setData(index, data, bytesToRead, 0, 0, 0, false);
    }
    void setDynamicData(size_t index, void* data, size_t dynamicIndex) {
	setData(index, data, 0, 0, 0, dynamicIndex, false);
    }
    /// Set the data of a shader buffer for all future frames
    void setAllData(size_t index, void* data) {
	setData(index, data, 0, 0, 0, 0, true);
    }
    void setAllData(size_t index, void* data, size_t bytesToRead) {
	setData(index, data, bytesToRead, 0, 0, 0, true);
    }
    /// Lowest level shader buffer writing function
    ///
    /// write up to `typeSize - destinationOffset` number of bytes.
    /// arrays and dynamics are not stored with C++ mem alignment.
    /// -> passing 0 for bytes to read will read (typeSize - destinationOffset) bytes
    /// -> Prints error if memory to write is out of range, or any indicies are out of range
    ///    returns without writing the data
    /// -> If the data passed is not large enough, it will segfault
    virtual void setData(
	    size_t index,
	    void* data,
	    size_t bytesToRead,
	    size_t destinationOffset,
	    size_t arrayIndex,
	    size_t dynamicIndex,
	    bool updateAllFrames) = 0;

    
    /// ------ Texture Samplers -------
    
    virtual void addTextureSamplers(size_t index,
				    size_t arrayCount,
				    std::vector<TextureSampler> samplers) = 0;
    
            void addTextureSamplers(size_t index,
				    std::vector<TextureSampler> samplers) {
		addTextureSamplers(index, samplers.size(), samplers);}
            void addTextureSamplers(size_t index,
				    TextureSampler sampler) {
		addTextureSamplers(index, 1, {sampler});}
    
    /// Update Texture Sampler - does not require any recreation
    virtual void updateSampler(size_t index, size_t arrayIndex, TextureSampler sampler) = 0;
            void updateSampler(size_t index, TextureSampler sampler)  {
		updateSampler(index, 0, sampler);}
    /// Get sampler descriptor stored at that binding index and array index
    virtual TextureSampler getSampler(size_t index, size_t arrayIndex) = 0;
            TextureSampler getSampler(size_t index) {
		return getSampler(index, 0); }

    /// ------ Textures -------

    virtual void addTextures(size_t index, size_t arrayCount,
			     std::vector<Resource::Texture> textures) = 0;
            void addTextures(size_t index, std::vector<Resource::Texture> textures) {
		addTextures(index, textures.size(), textures);}

    virtual void updateTextures(size_t index, size_t arrayIndex,
			       std::vector<Resource::Texture> textures) = 0;
            void updateTextures(size_t index, size_t arrayIndex,
			       Resource::Texture texture) {
		updateTextures(index, arrayIndex, std::vector<Resource::Texture>{texture});}
};

#endif /* GRAPHICS_API_SHADER_BUFFERS_H */
