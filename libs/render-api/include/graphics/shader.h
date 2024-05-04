#ifndef GRAPHICS_ENV_RENDER_SHADER_H
#define GRAPHICS_ENV_RENDER_SHADER_H

#include "resources.h"
#include <string>
#include <vector>

class PipelineInput {
public:
    enum class type {
	vec2,
	vec3,
	vec4,
	ivec4,
    };
    struct Entry {
	type input_type;
	float offset;	  
	Entry(type input_type, float offset) {
	    this->input_type = input_type;
	    this->offset = offset;
	}
      };
    
private:
    //float size;
    std::vector<Entry> entries;
};


enum class stageflag {
  vert = 0b01,
  frag = 0b10,
};

inline stageflag operator| (stageflag a, stageflag b) {
    return (stageflag)((int)a | (int)b);
}

struct TextureSampler {
    enum class filter {
	nearest = 0,
	linear,
    };
    enum class address_mode {
	repeat = 0,
	mirrored_repeat,
	clamp_to_edge,
	clamp_to_border,
    };

    TextureSampler() {}
    
    TextureSampler(filter filter, address_mode addressMode, int maxLod) {
	this->textureFilter = filter;
	this->addressMode = addressMode;
	this->maxLod = maxLod;
    }

    filter textureFilter = filter::linear;
    address_mode addressMode = address_mode::repeat;
    int maxLod = 1.0f;
};

class Set {
public:
    virtual size_t nextFreeIndex() = 0;

    /// ------ Shader Buffers -------
    
    virtual void addUniformBuffer(size_t index, size_t typeSize, size_t arrayCount) = 0;
            void addUniformBuffer(size_t index, size_t typeSize) {
		addUniformBuffer(index, typeSize, 1); }

    virtual void addStorageBuffer(size_t index, size_t typeSize, size_t arrayCount) = 0;
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

/// Holds shader sets
class ShaderPool {
public:
    virtual Set* CreateSet(stageflag flags) = 0;
    // automatically destroys created resources if already created
    virtual void CreateGpuResources() = 0;
    virtual void DestroyGpuResources() = 0;
};


class PushConstant {
private:
    //    stageflag stageFlags;
    //int offset; calc manually?
    //int size;
};
  
class PipelineLayout {
    
private:
    PipelineInput input;
    std::string vertexShader;
    std::string fragmentShader;
    std::vector<Set*> sets;  
    std::vector<PushConstant> pushConstants;
};

#endif
