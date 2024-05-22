#include <graphics/default_vertex_types.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <graphics/logger.h>

namespace vertex {

  ModelVertexType<data::Vertex2D> v2D( {
	  PipelineInput::Entry(
		  PipelineInput::type::vec3, offsetof(data::Vertex2D, Position)),
	  PipelineInput::Entry(
		  PipelineInput::type::vec2, offsetof(data::Vertex2D, TexCoord)) },
      [](ModelInfo::Vertex vert, glm::mat4 transform) {
	  data::Vertex2D v;
	  v.Position = transform * glm::vec4(vert.Position, 1.0f);
	  v.TexCoord = vert.TexCoord;
	  return v;
      });



  ModelVertexType<data::Vertex3D> v3D( {
	  PipelineInput::Entry(
		  PipelineInput::type::vec3, offsetof(data::Vertex3D, Position)),
	  PipelineInput::Entry(
		  PipelineInput::type::vec3, offsetof(data::Vertex3D, Normal)),
	  PipelineInput::Entry(
		  PipelineInput::type::vec2, offsetof(data::Vertex3D, TexCoord))},
      [](ModelInfo::Vertex vert, glm::mat4 transform) {
	  data::Vertex3D v;
	  v.Position = transform * glm::vec4(vert.Position, 1.0f);
	  v.Normal = glm::mat3(glm::inverseTranspose(transform)) * vert.Normal;
	  v.TexCoord = vert.TexCoord;
	  return v;
      });

  
  ModelVertexType<data::VertexAnim3D> Anim3D( {
	  PipelineInput::Entry(
		  PipelineInput::type::vec3, offsetof(data::VertexAnim3D, Position)),
	  PipelineInput::Entry(
		  PipelineInput::type::vec3, offsetof(data::VertexAnim3D, Normal)),
	  PipelineInput::Entry(
		  PipelineInput::type::vec2, offsetof(data::VertexAnim3D, TexCoord)),
	  PipelineInput::Entry(
		  PipelineInput::type::ivec4, offsetof(data::VertexAnim3D, BoneIDs)),
	  PipelineInput::Entry(
		  PipelineInput::type::vec4, offsetof(data::VertexAnim3D, Weights)),},
      [](ModelInfo::Vertex vert, glm::mat4 transform) {
	  data::VertexAnim3D v;
	  v.Position =  glm::vec4(vert.Position, 1.0f);
	  v.Normal = vert.Normal;
	  v.TexCoord = vert.TexCoord;
	  for(int vecElem = 0; vecElem < 4; vecElem++) {
	      if(vert.BoneIDs.size() <= vecElem) {
		  v.BoneIDs[vecElem] = -1;
		  v.Weights[vecElem] = 0;
	      } else {
		  v.BoneIDs[vecElem] = vert.BoneIDs[vecElem];
		  v.Weights[vecElem] = vert.BoneWeights[vecElem];
	      }
	  }
	  if(vert.BoneIDs.size() > 4)
	      LOG_CERR("vertex influenced by more than 4 bones, but only 4 bones will be used!\n",
		       "Warning");
	  return v;
      });
  
}
