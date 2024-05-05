#ifndef GRAPHICS_API_SHADER_INFO_H
#define GRAPHICS_API_SHADER_INFO_H

/// Enum flags for describing shader stages
/// 
/// Vertex and Fragment shaders supported
enum class shaderstage {
    vert = 1 << 0,
    frag = 1 << 1,
};

inline shaderstage operator| (shaderstage a, shaderstage b) {
    return (shaderstage)((int)a | (int)b);
}


/// A description of a Texture Sampler
/// 
/// for use in a shader buffer for reading from textures
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

    TextureSampler(filter filter, address_mode addressMode) {
	this->textureFilter = filter;
	this->addressMode = addressMode;
	this->maxLod = 1.0f;
    }

    filter textureFilter = filter::linear;
    address_mode addressMode = address_mode::repeat;
    int maxLod = 1.0f;
};

#endif /* GRAPHICS_API_SHADER_INFO_H */
