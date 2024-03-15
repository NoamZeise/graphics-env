#ifndef GLVERTEX_DATA_H
#define GLVERTEX_DATA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <resource_loader/vertex_types.h>

#include <vector>

class GLVertexData
{
public:
    GLVertexData() {}
    GLVertexData(std::vector<Vertex2D> &vertices, std::vector<unsigned int> &indices);
    GLVertexData(std::vector<Vertex3D> &vertices, std::vector<unsigned int> &indices);
    GLVertexData(std::vector<VertexAnim3D> &verticies, std::vector<unsigned int> &indices);
    ~GLVertexData();
    
    void Draw(unsigned int mode);
    void DrawInstanced(unsigned int mode, int count);
    void Draw(unsigned int mode, unsigned int verticies);

private:
    template <class T>
    void initBuffers(std::vector<T> &vertices, std::vector<unsigned int> &indices);
    
    GLuint VAO;
    GLuint VBO;
    GLuint EBO;
    GLuint size;
};



#endif
