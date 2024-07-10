#ifndef GRAPHICS_API_PIPELINE_H
#define GRAPHICS_API_PIPELINE_H

#include "shader_buffers.h"
#include "pipeline_input.h"

#include <string>
#include <vector>


class Pipeline {
public:

    static std::vector<char> ReadShaderCode(std::string path);
    
    Pipeline(PipelineInput input,
	     std::vector<char> vertexCode,
	     std::vector<char> fragmentCode);

    void addShaderSet(int setIndex, ShaderSet* set);

    void addPushConstant(shader::Stage stageFlags, size_t dataSize);

    void CreatePipeline();

    void DestroyPipeline();
protected:
    struct PushConstant {
	shader::Stage stageFlags;
	int dataSize;
    };
    
    PipelineInput input;
    std::vector<char> vertexShader;
    std::vector<char> fragmentShader;
    std::vector<bool> sets;  
    std::vector<PushConstant> pushConstants;
};

#endif /* GRAPHICS_API_PIPELINE_H */
