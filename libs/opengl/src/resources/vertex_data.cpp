#include "vertex_data.h"

template <class T>
void GLVertexData::initBuffers(std::vector<T> &vertices, std::vector<unsigned int> &indices) {
    this->size = (GLuint)indices.size();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

GLVertexData::GLVertexData(std::vector<Vertex2D> &vertices, std::vector<unsigned int> &indices) {
    initBuffers(vertices, indices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex2D), (void*)offsetof(Vertex2D, TexCoord));
}

GLVertexData::GLVertexData(std::vector<Vertex3D> &vertices, std::vector<unsigned int> &indices) {
    initBuffers(vertices, indices);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex3D), (void*)offsetof(Vertex3D, TexCoord));
}

#include <iostream>

GLVertexData::GLVertexData(std::vector<VertexAnim3D> &vertices, std::vector<unsigned int> &indices) {
    initBuffers(vertices, indices);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAnim3D),
			  (void*)offsetof(VertexAnim3D, Position));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAnim3D),
			  (void*)offsetof(VertexAnim3D, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexAnim3D),
			  (void*)offsetof(VertexAnim3D, TexCoord));
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, sizeof(VertexAnim3D),
			  (void*)offsetof(VertexAnim3D, BoneIDs));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(VertexAnim3D),
			  (void*)offsetof(VertexAnim3D, Weights));
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
