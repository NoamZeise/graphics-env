#ifndef GRAPHICS_API_PIPELINE_H
#define GRAPHICS_API_PIPELINE_H

#include "shader_buffers.h"
#include "pipeline_input.h"
#include <string>
#include <vector>

class Pipeline {
public:
    Pipeline(PipelineInput input,
	     std::string vertexCode,
	     std::string fragmentCode);

    void addShaderSet(ShaderSet* set);

    void addPushConstant(shader::Stage stageFlags, size_t dataSize);

    void CreatePipeline();

    void DestroyPipeline();
protected:
    struct PushConstant {
	shader::Stage stageFlags;
	int dataSize;
    };
    
    PipelineInput input;
    std::string vertexShader;
    std::string fragmentShader;
    std::vector<ShaderSet*> sets;  
    std::vector<PushConstant> pushConstants;
};

#endif /* GRAPHICS_API_PIPELINE_H */
