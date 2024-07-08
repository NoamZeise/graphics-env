#include "spirv_shaders.h"

#include <graphics/logger.h>
#include <spirv_glsl.hpp>
#include <fstream>
#include <stdint.h>

SpirvShaders::SpirvShaders(shader::PipelineSetup spirvShaders){
    shaders.push_back(
	    SpirvShaderGl(spirvShaders
			.getPath(shader::pipeline::_2D, shader::stage::vert)));
    shaders.push_back(
	    SpirvShaderGl(spirvShaders
			.getPath(shader::pipeline::_2D, shader::stage::frag)));

    shaders.push_back(
	    SpirvShaderGl("shaders/vulkan/test.frag.spv"));


    for(auto &s: shaders) {
	LOG("shader buffers:");
	for(int set = 0; set < s.sets.size(); set++) {
	    if(s.sets[set].bindings.size() > 0)
		LOG("Set " << set);
	    for(int b = 0; b < s.sets[set].bindings.size(); b++) {
		SpirvBindingGl* bind = &s.sets[set].bindings[b];
		LOG("Binding: " << b << " name: " <<
		    bind->name <<
		    " type: " <<
		    Binding::to_string(bind->type) <<
		    " count: " << bind->arrayCount);
	    }
	}
	for(int sb = 0; sb < s.combinedSamplers.size(); sb++) {
	    LOG("Combined Sampler: name: " <<
		s.combinedSamplers[sb].name <<
		" count: " << s.combinedSamplers[sb].arrayCount);
	}
	for(int pc = 0; pc < s.pushConsts.size(); pc++) {
	    LOG("Push Const Uniform - name: " << s.pushConsts[pc].name);
	}
	LOG(s.code);
	LOG_LINE();
    }
}

SpirvShaderGl::SpirvShaderGl(std::string path) {
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

    for(auto& uniform: resources.uniform_buffers) {
	addBinding(&uniform, &glsl, Binding::type::UniformBuffer);
    }

    for(auto& sb: resources.storage_buffers) {
	addBinding(&sb, &glsl, Binding::type::StorageBuffer);
    }

    for(auto& combinedIm: resources.sampled_images) {
	this->combinedSamplers.push_back(
		SpirvBindingGl(&combinedIm, &glsl, Binding::type::None));
    }

    for (auto &samplers : resources.separate_samplers) {
	addBinding(&samplers, &glsl, Binding::type::TextureSampler);
    }

    for (auto &images : resources.separate_images) {
	addBinding(&images, &glsl, Binding::type::Texture);
    }

    for(auto& pc: resources.push_constant_buffers) {
	this->pushConsts.push_back(
		SpirvBindingGl(&pc, &glsl, Binding::type::UniformBuffer));
    }
    
    spirv_cross::CompilerGLSL::Options options;
    options.version = 310;
    options.emit_push_constant_as_uniform_buffer = true;
    glsl.set_common_options(options);
    this->code = glsl.compile();
}

void SpirvShaderGl::addBinding(spirv_cross::Resource* res,
			     spirv_cross::CompilerGLSL* glsl,
			     Binding::type type) {
    unsigned int set_i = glsl->get_decoration(res->id, spv::DecorationDescriptorSet);
    unsigned int binding_i = glsl->get_decoration(res->id, spv::DecorationBinding);

    if(sets.size() <= set_i) {
	sets.resize(set_i + 1);
    }
    if(sets[set_i].bindings.size() <= binding_i) {
   	sets[set_i].bindings.resize(binding_i + 1);
    }
    sets[set_i].bindings[binding_i] = SpirvBindingGl(res, glsl, type);
}

SpirvBindingGl::SpirvBindingGl(spirv_cross::Resource* res,
			       spirv_cross::CompilerGLSL* glsl,
			       Binding::type type) {
    this->name = res->name;
    this->type = type;
    spirv_cross::SPIRType t = glsl->get_type(res->type_id);
    this->arrayCount = t.array.empty() ? 0 : t.array[0];
}
