#ifndef GRAPHICS_API_PIPELINE_H
#define GRAPHICS_API_PIPELINE_H

#include "shader_buffers.h"
#include "pipeline_input.h"

#include <string>
#include <vector>

// class RenderPass{};

class Pipeline {
public:

    struct Config {
	bool useDepthTest = true;	
    };
    

    static std::vector<char> ReadShaderCode(std::string path);

    void addShaderSet(int setIndex, ShaderSet* set);

    void addPushConstant(shader::Stage stageFlags, size_t dataSize);

    // Null pointer for now
    virtual void CreatePipeline(void* renderpass);

    virtual void DestroyPipeline();
    
protected:

    Pipeline(PipelineInput input,
	     std::vector<char> vertexShader,
	     std::vector<char> fragmentShader);
    
    struct PushConstant {
	shader::Stage stageFlags;
	size_t dataSize;
	size_t offset;
	PushConstant(shader::Stage stage, size_t dataSize, size_t offset);
    };
    
    PipelineInput input;
    std::vector<char> vertexShader;
    std::vector<char> fragmentShader;
    std::vector<ShaderSet*> sets;
    std::vector<PushConstant> pushConstants;
    bool created = false;
};

#endif /* GRAPHICS_API_PIPELINE_H */
