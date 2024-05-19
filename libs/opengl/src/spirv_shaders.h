#ifndef OGL_SPIRV_SHADERS_H
#define OGL_SPIRV_SHADERS_H

#include <graphics/shader_structs.h>
#include <render-internal/shader_buffers.h>
#include <vector>

namespace spirv_cross {
    struct Resource;
    struct CompilerGLSL;
}

struct SpirvBindingGl {
    SpirvBindingGl(){}
    SpirvBindingGl(spirv_cross::Resource* resource,
		   spirv_cross::CompilerGLSL* glsl,
		   Binding::type type);
    std::string name;
    Binding::type type = Binding::type::None;
    unsigned int arrayCount = 0;
};

struct SpirvDescriptorSetGl {
    std::vector<SpirvBindingGl> bindings;
};

struct SpirvShaderGl {
    SpirvShaderGl(std::string path);       
    
    std::vector<SpirvDescriptorSetGl> sets;
    std::vector<SpirvBindingGl> combinedSamplers;
    std::vector<SpirvBindingGl> pushConsts; 
    std::string code;

private:
    void addBinding(spirv_cross::Resource* resource,
		    spirv_cross::CompilerGLSL* glsl,
		    Binding::type type);
};

class SpirvShaders {
 public:
    SpirvShaders(shader::PipelineSetup spirvShaders);

 private:
    std::vector<SpirvShaderGl> shaders;
};


#endif /* OGL_SPIRV_SHADERS_H */
