#ifndef RESOURCES_H
#define RESOURCES_H

#include <glm/glm.hpp>
#include <string>

namespace Resource {

  struct Texture
  {
  	Texture()
  	{
  		path = "";
  		ID = 0;
  		dim = glm::vec2(1, 1);
  	}
  	Texture(unsigned int ID, glm::vec2 dimentions, std::string path)
  	{
  		this->path = path;
  		this->ID = ID;
  		this->dim = dimentions;
  	}
  	std::string path;
  	unsigned int ID = 0;
  	glm::vec2 dim = glm::vec2(0, 0);
  };

	struct Model
	{
		Model() { this->ID = 1000000; }
		Model(unsigned int ID) { this->ID = ID; }
		unsigned int ID;
	};

  struct Font
  {
    Font() { this->ID = 1000000; }
    Font(unsigned int ID)
    {
      this->ID = ID;
    }
    unsigned int ID;
  };

  struct QuadDraw
  {
	  QuadDraw(Texture tex, glm::mat4 model, glm::vec4 colour)
	  {
		  this->tex = tex;
		  this->model = model;
		  this->colour = colour;
	  }
	  Texture tex;
	  glm::mat4 model;
	  glm::vec4 colour;
  };

}

#endif
