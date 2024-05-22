#ifndef GLVERTEX_DATA_H
#define GLVERTEX_DATA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <graphics/pipeline_input.h>
#include <vector>

class GLVertexData
{
public:
    GLVertexData() {}
    GLVertexData(PipelineInput format,
		 void* vertexData,
		 uint32_t vertexCount,
		 std::vector<unsigned int> &indices);
    ~GLVertexData();
    
    void Draw(unsigned int mode);
    void DrawInstanced(unsigned int mode, int count);
    void Draw(unsigned int mode, unsigned int verticies);

private:
    void initBuffers(void* vertexData,
		     uint32_t vertexCount,
		     uint32_t vertexSize,
		     std::vector<unsigned int> &indices);
    
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint size;
};



#endif
