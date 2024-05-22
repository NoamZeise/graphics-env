#ifndef RENDER_API_PIPELINE_INPUT_H
#define RENDER_API_PIPELINE_INPUT_H

#include <vector>

/// Represents vertex input to a pipeline for a custom vertex type.
/// Constructed by ModelType
struct PipelineInput {
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
	bool operator==(Entry other) {
	    return input_type == other.input_type &&
		offset == other.offset;
	}
	bool operator!=(Entry other) {
	    return !(*this == other);
	}
    };

    PipelineInput(){}

    bool operator==(PipelineInput other) {
	if(entries.size() != other.entries.size())
	    return false;
	for(int i = 0; i < entries.size() ; i++)
	    if(entries[i] != other.entries[i])
		return false;
	return size == other.size;   
    }
    
    float size = 0;
    std::vector<Entry> entries;
};

#endif /* RENDER_API_PIPELINE_INPUT_H */
