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


const int SHADER_STAGE_COUNT = 2;
enum class stageflag {
  vert = 0b01,
  frag = 0b10,
};


class Set {
public:
    virtual size_t nextFreeIndex() = 0;
    
    virtual void addUniformBuffer(size_t index, size_t typeSize, size_t arrayCount) = 0;
    void addUniformBuffer(size_t index, size_t typeSize) { addUniformBuffer(index, typeSize, 1); }
    
    virtual void setData(size_t index, void* data) = 0;
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
