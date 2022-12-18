#include "vertex_data.h"

GLVertexData::GLVertexData(std::vector<GLVertex2D> &vertices, std::vector<unsigned int> &indicies)
{
	this->size = indicies.size();
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLVertex2D), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(unsigned int), indicies.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex2D), (void*)offsetof(GLVertex2D, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex2D), (void*)offsetof(GLVertex2D, texCoords));
}

GLVertexData::GLVertexData(std::vector<GLVertex3D> &vertices, std::vector<unsigned int> &indicies)
{
	this->size = indicies.size();
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLVertex3D), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies.size() * sizeof(unsigned int), indicies.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex3D), (void*)offsetof(GLVertex3D, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLVertex3D), (void*)offsetof(GLVertex3D, normal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(GLVertex3D), (void*)offsetof(GLVertex3D, texCoords));
}

GLVertexData::~GLVertexData()
{
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO);
}

void GLVertexData::Draw(unsigned int mode)
{
	glBindVertexArray(VAO);
	glDrawElements(mode, size, GL_UNSIGNED_INT, 0);
}

void GLVertexData::DrawInstanced(unsigned int mode, int count)
{
	glBindVertexArray(VAO);
	glDrawElementsInstanced(mode, size, GL_UNSIGNED_INT, 0, count);
}

void GLVertexData::Draw(unsigned int mode, unsigned int verticies)
{
	if (verticies > size)
		verticies = size;
	glBindVertexArray(VAO);
	glDrawElements(mode, verticies, GL_UNSIGNED_INT, 0);
}
