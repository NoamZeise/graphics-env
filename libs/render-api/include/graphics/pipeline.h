#ifndef GRAPHICS_API_PIPELINE_H
#define GRAPHICS_API_PIPELINE_H

#include "shader_buffers.h"
#include "pipeline_input.h"
#include "model/info.h"
#include <string>
#include <vector>

#include <functional>


/// Passed to Model Loader for loading models using a custom vertex type
/// and for creating pipelines that match your vertex type.
template <class T_Vert>
struct ModelType {

    /// -- entries --
    /// -> the individual entries of your vertex data
    ///
    /// i.e if your vertex data is defined as:
    ///      struct Vert { glm::vec3 pos, glm::vec3 norm }
    /// entries would be:
    ///      { PipelineInput::Entry(PipelineInput::type::vec3, offsetof(Vert, pos)),
    ///        PipelineInput::Entry(PipelineInput::type::vec3, offsetof(Vert, norm)) }
    ///
    /// ---------------------------------------------------------
    /// -- vertexLoader --
    /// -> takes a modelinfo vertex and the transform of the mesh
    ///    the vertex belongs to and returns a custom vertex type
    ///
    /// if there is no further mesh offsetting (ie animations)
    /// and your model has meshes with their own transforms, do
    ////   position = mesh transform * vec4(position, 1.0f)
    ///    normal = mat3(inverseTranspone(mesh transform)) * normal
    ///    similar for bitangent, binormal if using.
    ModelType(std::vector<PipelineInput::Entry> entries,
	      std::function<T_Vert(ModelInfo::Vertex, glm::mat4)> vertexLoader) {
	this->input.entries = entries;
	this->input.size = sizeof(T_Vert);
	this->vertexLoader = vertexLoader;
    }

    PipelineInput input;
    std::function<T_Vert(ModelInfo::Vertex, glm::mat4)> vertexLoader;
};

class PipelineLayout { 
private:
    PipelineInput input;
    std::string vertexShader;
    std::string fragmentShader;
    std::vector<ShaderSet*> sets;  
    std::vector<PushConstant> pushConstants;
};

#endif /* GRAPHICS_API_PIPELINE_H */
