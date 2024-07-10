#include <graphics/pipeline.h>
#include <fstream>
#include <stdexcept>

std::vector<char> Pipeline::ReadShaderCode(std::string path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in.is_open())
	throw std::runtime_error("failed to load shader at " + file);
    
    size_t fileSize = (size_t)in.tellg();
    std::vector<char> shaderCode(fileSize);
    
    in.seekg(0);
    in.read(shaderCode.data(), fileSize);
    in.close();
    return shaderCode;
}

Pipeline::Pipeline(PipelineInput input,
		   std::vector<char> vertexCode,
		   std::vector<char> fragmentCode) {
    this->input = input;
    this->vertexShader = vertexCode;
    this->fragmentShader = fragmentCode;
}

void Pipeline::addShaderSet(int setIndex, ShaderSet* set) {
    /// create desc layouts in vulkan override and destroy in destructor

    if(sets.size() <= setIndex)
	sets.resize(setIndex + 1, false);
    if(sets[setIndex]) {
	throw std::runtime_error(
		"tried to add multiple shader sets to same index in pipeline");
    }
    sets[setIndex] = true;
}
