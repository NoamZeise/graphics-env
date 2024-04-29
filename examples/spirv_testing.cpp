
#include <graphics/logger.h>
#include <spirv_glsl.hpp>
#include <fstream>
#include <stdint.h>

int main(int argc, char** argv) {
    LOG("SPIRV TESTING");
    LOG_LINE();
    if(argc < 2) {
	LOG("No file supplied");
	return 0;
    }
    
    std::ifstream in(argv[1], std::ios::binary | std::ios::ate);
    if (!in.is_open())
	throw std::runtime_error("failed to load shader at " + std::string(argv[1]));
    
    size_t fileSize = (size_t)in.tellg();
    std::vector<char> shaderCode(fileSize);
    
    in.seekg(0);
    in.read(shaderCode.data(), fileSize);
    in.close();

    //sample code from readme

    spirv_cross::CompilerGLSL glsl(reinterpret_cast<const uint32_t*>(
					   shaderCode.data()), shaderCode.size() / 4);

    glsl.build_combined_image_samplers();

    // The SPIR-V is now parsed, and we can perform reflection on it.
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();

    // Get all sampled images in the shader.
    for (auto &resource : resources.sampled_images)
	{
	    unsigned set = glsl.get_decoration(resource.id, spv::DecorationDescriptorSet);
	    unsigned binding = glsl.get_decoration(resource.id, spv::DecorationBinding);
	    printf("Image %s at set = %u, binding = %u\n", resource.name.c_str(), set, binding);

	    // Modify the decoration to prepare it for GLSL.
	    glsl.unset_decoration(resource.id, spv::DecorationDescriptorSet);
	}

    // Set some options.
    spirv_cross::CompilerGLSL::Options options;
    options.version = 310;
    options.es = true;
    glsl.set_common_options(options);

    // Compile to GLSL, ready to give to GL driver.
    std::string source = glsl.compile();

    LOG(source);
    
    return 0;
}
