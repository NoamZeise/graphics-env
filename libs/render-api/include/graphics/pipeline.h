#ifndef GRAPHICS_API_PIPELINE_H
#define GRAPHICS_API_PIPELINE_H

#include "shader_buffers.h"
#include "pipeline_input.h"
#include "render_pass.h"

#include <string>
#include <vector>

// class RenderPass{};

class Pipeline {
public:    

    enum class BlendOp {
	NoBlend,
	Add,
	Subtract,
	ReverseSubtract,
	Min,
	Max
    };
    
    enum class CullMode {
	None,
	Front,
	Back,
	Both,
    };
    
    struct Config {
	bool depthTest = true;
	bool sampleShading = false;
	BlendOp blendOp = BlendOp::Add;
	CullMode cullMode = CullMode::Front;
    };
    

    static std::vector<char> ReadShaderCode(std::string path);

    void addShaderLayout(int setIndex, ShaderSet* set);

    void addPushConstant(shader::Stage stageFlags, size_t dataSize);

    // Null pointer for now
    virtual void CreatePipeline(void* renderpass);

    virtual void DestroyPipeline();
    
    
protected:

    Pipeline(Config config,
	     PipelineInput input,
	     std::vector<char> vertexShader,
	     std::vector<char> fragmentShader);
    
    struct PushConstant {
	shader::Stage stageFlags;
	size_t dataSize;
	size_t offset;
	PushConstant(shader::Stage stage, size_t dataSize, size_t offset);
    };

    Config config;
    PipelineInput input;
    std::vector<char> vertexShader;
    std::vector<char> fragmentShader;
    std::vector<ShaderSet*> sets;
    std::vector<PushConstant> pushConstants;
    bool created = false;
};

#endif /* GRAPHICS_API_PIPELINE_H */
