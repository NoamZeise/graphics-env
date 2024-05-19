#include "spirv_shaders.h"

#include <graphics/logger.h>
#include <spirv_glsl.hpp>
#include <fstream>
#include <stdint.h>

SpirvShaders::SpirvShaders(shader::PipelineSetup spirvShaders){
    shaders.push_back(
	    SpirvShader(spirvShaders
			.getPath(shader::pipeline::_2D, shader::stage::vert)));
    shaders.push_back(
	    SpirvShader(spirvShaders
			.getPath(shader::pipeline::_2D, shader::stage::frag)));

    shaders.push_back(
	    SpirvShader("shaders/vulkan/test.frag.spv"));


    for(auto &s: shaders) {
	LOG("shader buffers:");
	for(int set = 0; set < s.sets.size(); set++) {
	    if(s.sets[set].bindings.size() > 0)
		LOG("Set " << set);
	    for(int b = 0; b < s.sets[set].bindings.size(); b++) {
		GLSpirvBinding* bind = &s.sets[set].bindings[b];
		LOG("Binding: " << b << " name: " <<
		    bind->name <<
		    " type: " <<
		    Binding::to_string(bind->type) <<
		    " count: " << bind->arrayCount);
		
	    }
	}
	LOG_LINE();
    }
}

SpirvShader::SpirvShader(std::string path) {
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in.is_open())
	throw std::runtime_error("failed to load shader at " + std::string(path));    
    size_t fileSize = (size_t)in.tellg();
    std::vector<char> shaderCode(fileSize);    
    in.seekg(0);
    in.read(shaderCode.data(), fileSize);
    in.close();
    spirv_cross::CompilerGLSL glsl(reinterpret_cast<const uint32_t*>(
					   shaderCode.data()), shaderCode.size() / 4);

    glsl.build_combined_image_samplers();
    
    spirv_cross::ShaderResources resources = glsl.get_shader_resources();
    
    for (auto &resource : resources.separate_samplers) {
	addBinding(&resource, &glsl, Binding::type::TextureSampler);
    }

    for (auto &resource : resources.separate_images) {
	addBinding(&resource, &glsl, Binding::type::Texture);
    }

    for(auto& uniform: resources.uniform_buffers) {
	glsl.set_decoration(uniform.id, spv::DecorationBufferBlock, true);
	addBinding(&uniform, &glsl, Binding::type::UniformBuffer);
    }

    for(auto& sb: resources.storage_buffers) {
	glsl.set_decoration(sb.id, spv::DecorationBufferBlock, true);
	addBinding(&sb, &glsl, Binding::type::StorageBuffer);
    }

    for(auto& sb: resources.sampled_images) {
	LOG("sampled image - set: " << glsl.get_decoration(sb.id, spv::DecorationDescriptorSet)
	    <<         " binding : "<< glsl.get_decoration(sb.id, spv::DecorationBinding));
    }
    
    spirv_cross::CompilerGLSL::Options options;
    options.version = 310;
    glsl.set_common_options(options);
    code = glsl.compile();
    LOG_LINE();
}

void SpirvShader::addBinding(spirv_cross::Resource* resource,
			     spirv_cross::CompilerGLSL* glsl,
			     Binding::type type) {
    unsigned int set_i = glsl->get_decoration(resource->id, spv::DecorationDescriptorSet);
    unsigned int binding_i = glsl->get_decoration(resource->id, spv::DecorationBinding);

    if(sets.size() <= set_i) {
	sets.resize(set_i + 1);
    }
    if(sets[set_i].bindings.size() <= binding_i) {
   	sets[set_i].bindings.resize(binding_i + 1);
    }
    GLSpirvBinding* b = &sets[set_i].bindings[binding_i];
    b->name = resource->name;
    b->type = type;
    spirv_cross::SPIRType t = glsl->get_type(resource->type_id);
    if(t.array.empty()) {
	b->arrayCount = 1;
    } else {
	b->arrayCount = t.array[0];
    }
}
