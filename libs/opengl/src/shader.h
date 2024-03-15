#ifndef GLSHADER_H
#define GLSHADER_H

#include <string>

namespace glenv {
  class GLShader {
  public:
      GLShader(const char* VertexShaderPath, const char* FragmentShaderPath);
      ~GLShader();
      void Use();
      unsigned int Location(const std::string& uniformName) const;
      unsigned int program() { return shaderProgram; }
  private:
      unsigned int shaderProgram = 0;
      unsigned int compileShader(const char* path, int shaderType);
  };
} //namespace

#endif
