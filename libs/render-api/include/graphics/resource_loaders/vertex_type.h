#ifndef RENDER_API_MODEL_VERTEX_TYPE_H
#define RENDER_API_MODEL_VERTEX_TYPE_H

#include "model_info.h"
#include "../pipeline_input.h"

#include <functional>

/// Passed to Model Loader for loading models using a custom vertex type
/// and for creating pipelines that match your vertex type.
template <class T_Vert>
struct ModelVertexType {

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
    ModelVertexType(std::vector<PipelineInput::Entry> entries,
	      std::function<T_Vert(ModelInfo::Vertex, glm::mat4)> vertexLoader) {
	this->input.entries = entries;
	this->input.size = sizeof(T_Vert);
	this->vertexLoader = vertexLoader;
    }

    PipelineInput input;
    std::function<T_Vert(ModelInfo::Vertex, glm::mat4)> vertexLoader;
};

#endif /* RENDER_API_MODEL_VERTEX_TYPE_H */
