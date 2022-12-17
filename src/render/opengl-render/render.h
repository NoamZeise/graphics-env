#ifndef OGL_RENDER_H
#define OGL_RENDER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include <stdexcept>
#include <iostream>
#include <string>
#include <atomic>
#include <vector>

#include "shader.h"
#include <config.h>
#include "resources/resources.h"
#include "resources/vertex_data.h"
#include "resources/texture_loader.h"
#include "resources/font_loader.h"
#include "resources/model_loader.h"

//match in shaders
const int MAX_2D_DRAWS = 10000;
const int MAX_3D_DRAWS = 10000;

const int MAX_2D_BATCH = 10000;
const int MAX_3D_BATCH = 10000;

class Render
{
public:
	Render(GLFWwindow* window, glm::vec2 target);
	~Render();
  static void SetGLFWWindowHints()
  {
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  }
  void set3DViewMatrixAndFov(glm::mat4 view, float fov, glm::vec4 camPos);
  void setLightDirection(glm::vec4 lightDir) {
    lighting.direction = lightDir;
  }
	Resource::Texture LoadTexture(std::string filepath);
  Resource::Model LoadModel(std::string filepath);
	Resource::Font LoadFont(std::string filepath);
  
  Resource::Model LoadAnimatedModel(std::string filepath, std::vector<Resource::ModelAnimation> *pGetAnimations) {
    std::cout << "skeletal animation not supported in ogl\n";
    
  }

    void BeginAnim3DDraw()
  {
std::cout << "skeletal animation not supported in ogl\n";
  }

  void DrawAnimModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix, Resource::ModelAnimation *animation)

  {
std::cout << "skeletal animation not supported in ogl\n";
  }

  
	void EndResourceLoad() { }

  void Begin2DDraw();
  void Begin3DDraw();
	void DrawModel(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMat);
	void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour, glm::vec4 texOffset);
	void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix, glm::vec4 colour);
	void DrawQuad(Resource::Texture texture, glm::mat4 modelMatrix);
	void DrawString(Resource::Font font, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour, float rotate);
	void DrawString(Resource::Font font, std::string text, glm::vec2 position, float size, float depth, glm::vec4 colour);
  void EndDraw(std::atomic<bool>& submit);

	void FramebufferResize();

private:

	void draw3DBatch(int drawCount, Resource::Model model);
	void draw2DBatch(int drawCount, Resource::Texture texture, glm::vec4 currentColour);

	struct LightingParameters
	{
		LightingParameters()
		{
			ambient = glm::vec4(1.0f, 1.0f, 1.0f, 0.35f);
    	diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
    	specular = glm::vec4(1.0f, 1.0f, 1.0f, 8.0f);
    	direction = glm::vec4(0.3f, -0.3f, -0.5f, 0.0f);
        camPos = glm::vec4(0.0f);
		}

		alignas(16) glm::vec4 ambient;
		alignas(16) glm::vec4 diffuse;
		alignas(16) glm::vec4 specular;
		alignas(16) glm::vec4 direction;
      alignas(16) glm::vec4 camPos;
	};

    LightingParameters lighting;

	GLFWwindow* window;
	glm::vec2 targetResolution;

  Shader* blinnPhongShader;
	Shader* flatShader;

  int width;
  int height;

  glm::mat4 proj2D;
  glm::mat4 view2D;

  glm::mat4 proj3D;
  glm::mat4 view3D;
	float fov;

  VertexData* quad;

  Resource::TextureLoader* textureLoader;
	Resource::FontLoader* fontLoader;
  Resource::ModelLoader* modelLoader;

  struct Draw2D
  {
    Draw2D() {}
    Draw2D(Resource::Texture tex, glm::mat4 model, glm::vec4 colour, glm::vec4 texOffset);
    Resource::Texture tex;
    glm::mat4 model;
    glm::vec4 colour;
    glm::vec4 texOffset;
  };
  struct Draw3D
  {
    Draw3D() {}
    Draw3D(Resource::Model model, glm::mat4 modelMatrix, glm::mat4 normalMatrix);

    Resource::Model model;
    glm::mat4 modelMatrix;
    glm::mat4 normalMatrix;
  };

  unsigned int current2DDraw = 0;
  Draw2D draw2DCalls[MAX_2D_DRAWS];
  unsigned int current3DDraw = 0;
  Draw3D draw3DCalls[MAX_3D_DRAWS];

	glm::mat4 perInstance2DModel[MAX_2D_BATCH];
	glm::vec4 perInstance2DTexOffset[MAX_2D_BATCH];
	GLuint model2DSSBO;
	GLuint texOffset2DSSBO;

	glm::mat4 perInstance3DModel[MAX_3D_BATCH];
	glm::mat4 perInstance3DNormal[MAX_3D_BATCH];
	GLuint model3DSSBO;
	GLuint normal3DSSBO;
};





#endif
