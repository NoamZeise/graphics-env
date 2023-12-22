#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>

#include "input.h"
#include "timer.h"

namespace camera {

  class Base {
  public:
      Base() {}
      virtual glm::mat4 getView() = 0;
      virtual glm::vec3 getPos() = 0;
  };
  
  class FirstPerson : public Base {
  public:
      FirstPerson() : Base() {};
      FirstPerson(glm::vec3 position);

      void control(glm::vec2 ctrlDir);
      /// simple camera for flying around the world
      void flycamUpdate(Input &input, Timer &timer);
      
      glm::mat4 getView() override;
      glm::vec3 getPos() override;
      void setPos(glm::vec3 pos);

      float inputSensitivity = 1.0f;
      
  private:
      void calculateVectors();
      
      glm::vec3 pos;
      glm::vec3 worldUp = glm::vec3(0.0f, 0.0f, 1.0f);

      float yaw = 200.0f;
      float pitch = -20.0f;

      glm::vec3 forward;
      glm::vec3 left;
      glm::vec3 up;
      glm::mat4 view = glm::mat4(1.0f);
  };

  class ThirdPerson : public Base {
  public:

      ThirdPerson();
      void control(glm::vec2 ctrlDir);
      void setTarget(glm::vec3 target, float radius);
      
      glm::mat4 getView() override;
      glm::vec3 getPos() override;
      /// set pos in relation to target (ie camPos = pos + target).
      /// normalizes given pos
      void      setPos(glm::vec3 pos);
      glm::vec3 getWorldUp();
      /// must be non-zero
      void      setWorldUp(glm::vec3 worldUp);
      glm::vec3 getTargetForward();
      glm::vec3 getTargetLeft();

      /// a constant that ctrlDir is multiplied by
      float inputSensitivity = 1.0f;
      /// how close can the camera get to the top/bottom
      /// of the target. Range is [0, 1].
      float camlimit = 0.9; 
      
  private:
      void updateView();
    
      glm::vec3 worldUp = glm::normalize(glm::vec3(0, 0, 1));

      // local space props
      glm::vec3 pos;
      glm::vec3 forward, up, left;

      float radius = 1.0f;
      glm::vec3 target;
      
      glm::mat4 targetMat = glm::mat4(1.0f);
      glm::vec3 worldPos;
      glm::mat4 view;
      glm::vec3 targetForward;
      glm::vec3 targetLeft;
  };
  
} // namesapce end

#endif
