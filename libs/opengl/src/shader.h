#ifndef GLSHADER_H
#define GLSHADER_H

#include <string>

namespace glenv {
  class GLShader {
  public:
      GLShader(std::string VertexShaderPath, std::string FragmentShaderPath);
      ~GLShader();
      void Use();
      unsigned int Location(const std::string& uniformName) const;
      unsigned int program() { return shaderProgram; }
  private:
      unsigned int shaderProgram = 0;
      unsigned int compileShader(std::string path, int shaderType);
  };
} //namespace

#endif
