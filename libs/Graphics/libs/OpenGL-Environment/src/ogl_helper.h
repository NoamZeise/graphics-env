#ifndef OGL_HELPER_H
#define OGL_HELPER_H

#include <glad/glad.h>
#include <glm/glm.hpp>

#define sizeAndPtr(arr) sizeof(arr), &arr

namespace ogl_helper
{
  void createShaderStorageBuffer(GLuint* glBuffer, size_t bufferSize, void* pBufferArray)
  {
    glGenBuffers(1, glBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *glBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, pBufferArray, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  void shaderStorageBufferData(GLuint glBuffer, size_t bufferSize, void* pBufferArray, GLuint bufferBaseSize)
  {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, pBufferArray, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferBaseSize, glBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER,0 );
  }
}

#endif
