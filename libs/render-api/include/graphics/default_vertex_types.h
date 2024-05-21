#ifndef RENDER_API_DEFAULT_VERTEX_TYPES_H
#define RENDER_API_DEFAULT_VERTEX_TYPES_H

#include "pipeline.h"
#include <glm/glm.hpp>

struct Vertex2D {
    glm::vec3 Position;
    glm::vec2 TexCoord;
};
extern ModelType<Vertex2D> vert2DType;

struct Vertex3D {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoord;
};
extern ModelType<Vertex3D> vert3DType;

struct VertexAnim3D {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoord;
    glm::ivec4 BoneIDs;
    glm::vec4  Weights;
};
extern ModelType<VertexAnim3D> vertAnim3DType;

#endif /* RENDER_API_DEFAULT_VERTEX_TYPES_H */
