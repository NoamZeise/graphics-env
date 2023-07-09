#ifndef VECTOR_MATHS_H
#define VECTOR_MATHS_H

#include <glm/glm.hpp>
#include <string>
#include <stdio.h>
#include <random>
#include <ctime>

namespace gh
{
  inline bool colliding(glm::vec4 a, glm::vec4 b)
  {
    return  a.x < b.x + b.z &&
      a.x + a.z > b.x &&
      a.y < b.y + b.w &&
      a.y + a.w > b.y;
  }

inline bool aInB(glm::vec4 a, glm::vec4 b)
{
return a.x > b.x && a.x < b.x + b.z &&
  a.y > b.y && a.y < b.y + b.z;
}

inline bool contains(glm::vec2 p, glm::vec4 r)
{
return r.x < p.x && p.x < r.x + r.z  && r.y < p.y && p.y < r.y + r.w;
}

  inline void addVec2ToRect(glm::vec2 v, glm::vec4 *r)
  {
    *r = glm::vec4(r->x + v.x, r->y + v.y, r->z, r->w);
  }

inline bool exists (const std::string& name) {
if (FILE *file = fopen(name.c_str(), "r")) {
fclose(file);
return true;
} else {
return false;
}
}

    inline glm::vec2 centre(glm::vec4 rect)
    {

	return glm::vec2(rect.x + rect.z/2, rect.y + rect.w/2);
    }

inline float max(float a, float b)
{
return a > b ? a : b;
}

inline float min(float a, float b)
{
return a < b ? a : b;
}

class Random
{
public:
Random()
{
randomGen = std::mt19937((unsigned int)time(0));
		posReal = std::uniform_real_distribution<float>(0, 1);
		real = std::uniform_real_distribution<float>(-1, 1);
	}

	float Real()
	{
		return real(randomGen);
	}
	float PositiveReal()
	{
		return posReal(randomGen);
	}
    std::string stringNum(int max)
    {
	return std::to_string((int)(PositiveReal()*max) + 1);
    }
    std::string stringNum(int min, int max)
    {
	return std::to_string((int)(PositiveReal()*max) + min);
    }
private:
	std::mt19937 randomGen;
	std::uniform_real_distribution<float> posReal;
	std::uniform_real_distribution<float> real;
};

  
}
  
#endif
