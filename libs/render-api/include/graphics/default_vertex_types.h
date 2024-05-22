#ifndef RENDER_API_DEFAULT_VERTEX_TYPES_H
#define RENDER_API_DEFAULT_VERTEX_TYPES_H

#include "pipeline.h"
#include <glm/glm.hpp>

namespace vertex {

    namespace data {
	struct Vertex2D {
	    glm::vec3 Position;
	    glm::vec2 TexCoord;
	};

	struct Vertex3D {
	    glm::vec3 Position;
	    glm::vec3 Normal;
	    glm::vec2 TexCoord;
	};

	struct VertexAnim3D {
	    glm::vec3 Position;
	    glm::vec3 Normal;
	    glm::vec2 TexCoord;
	    glm::ivec4 BoneIDs;
	    glm::vec4  Weights;
	};
    }

    
    extern ModelType<data::Vertex2D> v2D;

    extern ModelType<data::Vertex3D> v3D;

    extern ModelType<data::VertexAnim3D> Anim3D;

}

#endif /* RENDER_API_DEFAULT_VERTEX_TYPES_H */
