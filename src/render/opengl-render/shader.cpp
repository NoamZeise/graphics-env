#include "shader.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>

namespace glenv {

GLShader::GLShader(const char* VertexShaderPath, const char* FragmentShaderPath)
{
//create shader
	unsigned int vShader, fShader;

	vShader = compileShader(VertexShaderPath, false);
	fShader = compileShader(FragmentShaderPath, true);

	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vShader);
	glAttachShader(shaderProgram, fShader);
	glLinkProgram(shaderProgram);

	int isLinked = 0;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
	if (!isLinked)
	{
		int logSize = 0;
		glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

		char* errorLog = new char[logSize];
		glGetShaderInfoLog(shaderProgram, logSize, &logSize, errorLog);

		std::cout << "failed to link shader program\n" << errorLog << std::endl;

		delete[] errorLog;
		errorLog = nullptr;
		glDeleteProgram(shaderProgram);
	}
	glDetachShader(shaderProgram, vShader);
	glDetachShader(shaderProgram, fShader);
	glDeleteShader(vShader);
	glDeleteShader(fShader);
}

GLShader::~GLShader()
{
	glDeleteProgram(shaderProgram);
}

unsigned int GLShader::compileShader(const char* path, bool isFragmentShader)
{
	std::string dir = path;
	unsigned int shader;
	if (isFragmentShader)
		shader = glCreateShader(GL_FRAGMENT_SHADER);
	else
		shader = glCreateShader(GL_VERTEX_SHADER);

	std::ifstream in(path);
	std::string shaderSource((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
	in.close();
	const char* source = shaderSource.c_str();
	
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	int isCompiled;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (!isCompiled)
	{
		int logSize = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

		char* errorLog = new char[logSize];
		glGetShaderInfoLog(shader, logSize, &logSize, errorLog);

		std::cerr << "failed to compile shader " << path << "\n" << errorLog << std::endl;

		delete[] errorLog;
		errorLog = nullptr;
		glDeleteShader(shader);
	}

	return shader;
}

void GLShader::Use()
{
	glUseProgram(shaderProgram);
}

unsigned int GLShader::Location(const std::string& uniformName) const
{
	return glGetUniformLocation(shaderProgram, uniformName.c_str());
}

}//namespace
