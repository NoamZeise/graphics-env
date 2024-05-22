#include "vertex_data.h"
#include <stdexcept>

void GLVertexData::initBuffers(void* vertexData,
			       uint32_t vertexCount,
			       uint32_t vertexSize,
			       std::vector<unsigned int> &indices) {
    this->size = (GLuint)indices.size();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, vertexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		 indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

GLVertexData::GLVertexData(PipelineInput format,
			   void* vertexData,
			   uint32_t vertexCount,
			   std::vector<unsigned int> &indices) {
    size_t size = format.size;
    initBuffers(vertexData, vertexCount, size, indices);
    for(int i = 0; i < format.entries.size(); i++) {
	glEnableVertexAttribArray(i);
	size_t offset = format.entries[i].offset;
	
	switch(format.entries[i].input_type) {
	case PipelineInput::type::vec2:
	    glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, size, (void*)offset);
	    break;
	case PipelineInput::type::vec3:
	    glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, size, (void*)offset);
	    break;
	case PipelineInput::type::vec4:
	    glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, size, (void*)offset);
	    break;
	case PipelineInput::type::ivec4:
	    glVertexAttribIPointer(i, 4, GL_INT, size, (void*)offset);
	    break;
	default:
	    throw std::runtime_error(
		    "GL Vertex Data: Unrecognised pipeline input entry type!");
	}
    }
}

GLVertexData::~GLVertexData() {
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void GLVertexData::Draw(unsigned int mode) {
    glBindVertexArray(VAO);
    glDrawElements(mode, size, GL_UNSIGNED_INT, 0);
}

void GLVertexData::DrawInstanced(unsigned int mode, int count) {
    glBindVertexArray(VAO);
    glDrawElementsInstanced(mode, size, GL_UNSIGNED_INT, 0, count);
}

void GLVertexData::Draw(unsigned int mode, unsigned int verticies) {
    if (verticies > size)
	verticies = size;
    glBindVertexArray(VAO);
    glDrawElements(mode, verticies, GL_UNSIGNED_INT, 0);
}
