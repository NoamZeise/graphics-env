#include "shader.h"

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>

#include <graphics/logger.h>

namespace glenv {

  GLShader::GLShader(std::string VertexShaderPath, std::string FragmentShaderPath) {
      //create shader
      unsigned int vShader, fShader;

      vShader = compileShader(VertexShaderPath, GL_VERTEX_SHADER);
      fShader = compileShader(FragmentShaderPath, GL_FRAGMENT_SHADER);

      shaderProgram = glCreateProgram();
      glAttachShader(shaderProgram, vShader);
      glAttachShader(shaderProgram, fShader);
      glLinkProgram(shaderProgram);

      int isLinked = 0;
      glGetProgramiv(shaderProgram, GL_LINK_STATUS, &isLinked);
      if (!isLinked) {
	  int logSize = 0;
	  glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logSize);

	  char* errorLog = new char[logSize];
	  glGetShaderInfoLog(shaderProgram, logSize, &logSize, errorLog);

	  LOG_ERROR("failed to link shader program\n" << errorLog);

	  delete[] errorLog;
	  errorLog = nullptr;
	  glDeleteProgram(shaderProgram);
      }
      glDetachShader(shaderProgram, vShader);
      glDetachShader(shaderProgram, fShader);
      glDeleteShader(vShader);
      glDeleteShader(fShader);
  }

  GLShader::~GLShader() {
      glDeleteProgram(shaderProgram);
  }

  unsigned int GLShader::compileShader(std::string path, int shaderType) {
      std::string dir = path;
      unsigned int shader = glCreateShader(shaderType);

      std::ifstream in(path);
      std::string shaderSource((std::istreambuf_iterator<char>(in)),
			       std::istreambuf_iterator<char>());
      in.close();
      const char* source = shaderSource.c_str();
	
      glShaderSource(shader, 1, &source, NULL);
      glCompileShader(shader);

      int isCompiled;
      glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
      if(!isCompiled) {
	  int logSize = 0;
	  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);

	  char* errorLog = new char[logSize];
	  glGetShaderInfoLog(shader, logSize, &logSize, errorLog);

	  LOG_ERROR("failed to compile shader " << path << "\n" << errorLog);

	  delete[] errorLog;
	  errorLog = nullptr;
	  glDeleteShader(shader);
      }

      return shader;
  }

  void GLShader::Use() {
      glUseProgram(shaderProgram);
  }

  unsigned int GLShader::Location(const std::string& uniformName) const {
      return glGetUniformLocation(shaderProgram, uniformName.c_str());
  }

}//namespace
