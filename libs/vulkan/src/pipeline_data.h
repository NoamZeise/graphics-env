/// The input data descriptions for the default graphics pipelines.
/// Vertex2D, Vertex3D, VertexAnim3D bindings and attribute descriptions.

#ifndef VKENV_PIPELINE_DATA_H
#define VKENV_PIPELINE_DATA_H

#include <glm/glm.hpp>
#include <volk.h>

struct fragPushConstants {
    glm::vec4 colour;
    glm::vec4 texOffset;
    int32_t TexID;
};

#endif
