#ifndef OGL_SPIRV_SHADERS_H
#define OGL_SPIRV_SHADERS_H

#include <graphics/shader_structs.h>
#include <render-internal/shader_buffers.h>
#include <vector>

struct GLSpirvBinding {
    std::string name;
    Binding::type type = Binding::type::None;
    unsigned int arrayCount = 0;
};

struct GLSpirvDescriptorSet {
    std::vector<GLSpirvBinding> bindings;
};

namespace spirv_cross {
    struct Resource;
    struct CompilerGLSL;
}

struct SpirvShader {
    SpirvShader(std::string path);
    
    void addBinding(spirv_cross::Resource* resource,
		    spirv_cross::CompilerGLSL* glsl,
		    Binding::type type);
    
    std::vector<GLSpirvDescriptorSet> sets;
    std::string code;
};

class SpirvShaders {
 public:
    SpirvShaders(shader::PipelineSetup spirvShaders);

 private:
    std::vector<SpirvShader> shaders;
};


#endif /* OGL_SPIRV_SHADERS_H */
