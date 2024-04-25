#include <glm/glm.hpp>
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
    float size;
    std::vector<Entry> entries;
};

// Pass to Set to create a binding
class Binding {
public:
    Binding UniformBuffer(int typeSize, int arrayCount) {
	return Binding(type::UniformBuffer, typeSize, arrayCount, 1);
    }
    Binding UniformBuffer(int typeSize) {
	return Binding(type::UniformBuffer, typeSize, 1, 1);
    }
    // Dynamic -> swap part of buffer used between draws
    enum class type {
	UniformBuffer,
	UniformBufferDynamic,
	StorageBuffer,
	StorageBufferDynamic,
	TextureSampler,
	Texture,
    };
      
private:
    Binding(type binding_type,
	    size_t typeSize,
	    size_t arrayCount,
	    size_t dynamicCount) {
	this->binding_type = binding_type;
	this->typeSize = typeSize;
	this->arrayCount = arrayCount;
	this->dynamicCount = dynamicCount;
    }
    type binding_type;
    size_t typeSize;
    size_t arrayCount;
    size_t dynamicCount;
};

const int SHADER_STAGE_COUNT = 2;
enum class stageflag {
  vert = 0b01,
  frag = 0b10,
};

class Set {
public:
    Set(stageflag stageFlags) {
	this->stageFlags = stageFlags;
    }
      
private:
    stageflag stageFlags;
    std::vector<Binding> bindings;
};


class PushConstant {
private:
    stageflag stageFlags;
    //int offset; calc manually?
    int size;
};
  
class PipelineLayout {

private:
    PipelineInput input;
    std::string vertexShader;
    std::string fragmentShader;
    std::vector<Set> sets;  
    std::vector<PushConstant> pushConstants;
};
