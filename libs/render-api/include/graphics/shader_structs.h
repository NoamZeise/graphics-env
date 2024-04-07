#ifndef OUT_GRAPHICS_SHADER_STRUCTS
#define OUT_GRAPHICS_SHADER_STRUCTS

#include <glm/glm.hpp>
#include <string>

namespace shader {
    const int PIPELINE_COUNT = 4;
    enum class pipeline {
	_2D = 0,
	_3D,
	anim3D,
	final,
    };
    const int SHADER_STAGE_COUNT = 2;
    enum class stage {
	vert = 0,
	frag,
    };

    class PipelineSetup {
    public:
	void setPath(pipeline pipeline, stage stage, std::string path) {
	    shader_path[(int)pipeline][(int)stage] = path;
	}
	std::string getPath(pipeline pipeline, stage stage) {
	    return shader_path[(int)pipeline][(int)stage];
	}

    private:
	std::string shader_path[PIPELINE_COUNT][SHADER_STAGE_COUNT];
    };
}

struct BPLighting {
  BPLighting() {
    ambient = glm::vec4(1.0f, 1.0f, 1.0f, 0.35f);
    diffuse = glm::vec4(1.0f, 1.0f, 1.0f, 0.8f);
    specular = glm::vec4(1.0f, 1.0f, 1.0f, 8.0f);
    direction = glm::vec4(0.3f, 0.3f, -0.5f, 0.0f);
  }

  alignas(16) glm::vec4 ambient;
  alignas(16) glm::vec4 diffuse;
  alignas(16) glm::vec4 specular;
  alignas(16) glm::vec4 direction;
  alignas(16) glm::vec4 camPos;
};

#endif
