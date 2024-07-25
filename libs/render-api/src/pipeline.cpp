#include <graphics/pipeline.h>
#include <fstream>
#include <stdexcept>

std::vector<char> Pipeline::ReadShaderCode(std::string path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in.is_open())
	throw std::runtime_error("failed to load shader at " + path);
    
    size_t fileSize = (size_t)in.tellg();
    std::vector<char> shaderCode(fileSize);
    
    in.seekg(0);
    in.read(shaderCode.data(), fileSize);
    in.close();
    return shaderCode;
}

void Pipeline::addShaderLayout(int setIndex, ShaderSet* set) {
    /// create desc layouts in vulkan override and destroy in destructor

    if(sets.size() <= setIndex)
	sets.resize(setIndex + 1, nullptr);
    if(sets[setIndex]) {
	throw std::runtime_error(
		"tried to add multiple shader sets to same index in pipeline");
    }
    sets[setIndex] = set;
}

Pipeline::PushConstant::PushConstant(shader::Stage stage, size_t dataSize, size_t offset) {
    this->stageFlags = stage;
    this->dataSize = dataSize;
    this->offset = offset;    
}

void Pipeline::addPushConstant(shader::Stage stageFlags, size_t dataSize) {
    size_t offset = 0;
    for(int i = 0; i < pushConstants.size(); i++) {
	if((pushConstants[i].stageFlags & stageFlags) != 0)
	    throw std::runtime_error(
		    "Pipeline Setup: each shader stage can only have one set of push constants!");
	if(pushConstants[i].stageFlags & shader::Stage::vert)
	    offset = pushConstants[i].dataSize;
    }
    if((stageFlags & shader::Stage::vert) != 0)
	offset = 0;
    else if((stageFlags & shader::Stage::frag) == 0)
	throw std::runtime_error(
		"Pipeline Setup: push constant set did not have vertex or fragment stage flags");
    pushConstants.push_back(PushConstant(stageFlags, dataSize, offset));
}

void Pipeline::CreatePipeline(void *renderpass) { created = true; }

void Pipeline::DestroyPipeline() { created = false; }


/// ---- protected fns ----


Pipeline::Pipeline(Config config,
		   PipelineInput input,
		   std::vector<char> vertexShader,
		   std::vector<char> fragmentShader) {
    this->config = config;
    this->input = input;
    this->vertexShader = vertexShader;
    this->fragmentShader = fragmentShader;
}
