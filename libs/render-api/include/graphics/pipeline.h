#ifndef GRAPHICS_API_PIPELINE_H
#define GRAPHICS_API_PIPELINE_H

#include "shader_buffers.h"
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

class PipelineLayout {    
private:
    PipelineInput input;
    std::string vertexShader;
    std::string fragmentShader;
    std::vector<ShaderSet*> sets;  
    std::vector<PushConstant> pushConstants;
};

#endif /* GRAPHICS_API_PIPELINE_H */
