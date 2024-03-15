#include "ogl_helper.h"
#include <graphics/logger.h>

namespace ogl_helper {
  
void createShaderStorageBuffer(GLuint* glBuffer, size_t bufferSize, void* pBufferArray) {
    glGenBuffers(1, glBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *glBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, pBufferArray, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

 void shaderStorageBufferData(GLuint glBuffer, size_t bufferSize, void* pBufferArray,
			      GLuint bufferBaseSize){
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, glBuffer);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, pBufferArray, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufferBaseSize, glBuffer);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  /// 1 samples = no multisamping
  GLuint genTexture(GLuint format, GLsizei width, GLsizei height, unsigned char* data,
		    bool mipmapping, int filtering, int addressingMode, unsigned int samples) {
      GLuint texture;
      glGenTextures(1, &texture);
      int texType = samples > 1 ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
      glBindTexture(texType, texture);
      if(samples > 1)
	  glTexImage2DMultisample(texType, samples, format, width, height, GL_FALSE);
      else
	  glTexImage2D(texType, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

      if(mipmapping)
	  glGenerateMipmap(texType);

      if(samples == 1) {
	  glTexParameteri(texType, GL_TEXTURE_WRAP_S, addressingMode);
	  glTexParameteri(texType, GL_TEXTURE_WRAP_T, addressingMode);

	  glTexParameteri(texType, GL_TEXTURE_MIN_FILTER, filtering);
	  glTexParameteri(texType, GL_TEXTURE_MAG_FILTER, filtering);
      }

      glBindTexture(texType, 0);
      return texture;
  }
}
