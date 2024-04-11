#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace pipeline {
  class Input {
  public:
      enum class type {
	  vec2,
	  vec3,
	  vec4,
	  ivec4,
      };
      struct Entry {
	  type input_type;
	  float offset;	  
	  Entry(type input_type, float offset) {
	      this->input_type = input_type;
	      this->offset = offset;
	  }
      };
      
  private:
      float size;
      std::vector<Entry> entries;
  };

  class Binding {
  public:
      enum class type {
	  UniformBuffer,
	  UniformBufferDynamic,
	  StorageBuffer,
	  StorageBufferDynamic,
	  TextureSampler,
	  Texture,
       };
      Binding(type binding_type);
      
  private:
      void init(type binding_type,
		size_t typeSize,
		size_t arrayCount,
		size_t dynamicCount);
      type binding_type;
  };

  const int SHADER_STAGE_COUNT = 2;
  enum class stage {
      vert = 0,
      frag,
  };
  
  class Set {
  public:
      Set(stage shader_stage) {
	  this->shader_stage = shader_stage;
      }
      void addBinding(Binding binding) {
	  bindings.push_back(binding);
      }
      
  private:
      stage shader_stage;
      std::vector<Binding> bindings;
  };

  
  class Pipeline {

  private:
      Input input;
      std::string shader[SHADER_STAGE_COUNT];
      std::vector<Set> sets;     
      std::vector<Binding> push_constants;
  };
  
}
