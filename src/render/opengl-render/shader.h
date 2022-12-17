#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

class Shader
{
public:
	Shader() {}
	Shader(const char* VertexShaderPath, const char* FragmentShaderPath);
	~Shader();
	void Use();
	unsigned int Location(const std::string& uniformName) const;
	unsigned int program() { return shaderProgram; }

private:
	unsigned int shaderProgram = 0;
	unsigned int compileShader(const char* path, bool isFragmentShader);
};


#endif // !SHADER_H
