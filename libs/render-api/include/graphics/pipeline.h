#ifndef GRAPHICS_API_PIPELINE_H
#define GRAPHICS_API_PIPELINE_H

#include "shader_buffers.h"
#include "pipeline_input.h"
#include <string>
#include <vector>

class PipelineLayout {    
private:
    PipelineInput input;
    std::string vertexShader;
    std::string fragmentShader;
    std::vector<ShaderSet*> sets;  
    std::vector<PushConstant> pushConstants;
};

#endif /* GRAPHICS_API_PIPELINE_H */
