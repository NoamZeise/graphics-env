#ifndef GRAPHICS_API_SHADER_BUFFERS_H
#define GRAPHICS_API_SHADER_BUFFERS_H

#include "resources.h"
#include "shader_info.h"
#include <vector>

class PushConstant {
private:
    shaderstage stageFlags;
    //int offset; calc manually?
    //int size;
};

class Set;

/// Holds shader sets
class ShaderPool {
public:
    virtual Set* CreateSet(shaderstage flags) = 0;
    // automatically destroys created resources if already created
    virtual void CreateGpuResources() = 0;
    virtual void DestroyGpuResources() = 0;
};

class Set {
public:    
    /// ------ Shader Buffers -------
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
    
    void setData(size_t index, void* data) {
	setData(index, data, 0, 0, 0, 0);
    }
    void setData(size_t index, void* data, size_t bytesToRead) {
	setData(index, data, bytesToRead, 0, 0, 0);
    }
    /// Lowest level shader buffer writing function
    ///
    /// Set up to `typeSize - destinationOffset` number of bytes
    /// arrays and dynamics are not stored with C++ mem alignment.
    /// -> passing 0 for bytes to read will read (typeSize - destinationOffset) bytes
    /// -> Prints error if memory to write is out of range, or any indicies are out of range
    ///    returns without writing the data
    virtual void setData(
	    size_t index,
	    void* data,
	    size_t bytesToRead,
	    size_t destinationOffset,
	    size_t arrayIndex,
	    size_t dynamicIndex) = 0;

    
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
		updateTextures(index, arrayIndex, {texture});}
};

#endif /* GRAPHICS_API_SHADER_BUFFERS_H */
