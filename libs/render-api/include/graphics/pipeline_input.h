#ifndef GRAPHICS_API_PIPELINE_INPUT_H
#define GRAPHICS_API_PIPELINE_INPUT_H

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

#endif /* GRAPHICS_API_PIPELINE_INPUT_H */
