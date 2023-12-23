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
      glm::mat4 getView() { return view; };
      glm::vec3 getPos() { return pos; };

      /// needs to be a normalized vector
      glm::vec3 worldUp = glm::vec3(0, 0, 1);
  protected:
      glm::vec3 pos;
      glm::vec3 forward, left, up;
      /// calc camera basis vectors using forwards vector and given up
      /// also updates view
      void calcBasis();
      void updateView();
  private:
      glm::mat4 view;
  };
  
  class FirstPerson : public Base {
  public:
      FirstPerson();
      FirstPerson(glm::vec3 position);
      void control(glm::vec2 ctrlDir);
      /// simple camera for flying around the world
      void flycamUpdate(Input &input, Timer &timer);
      void setPos(glm::vec3 pos);
  };

  class ThirdPerson : public Base {
  public:
      ThirdPerson();
      void control(glm::vec2 ctrlDir);
      void setTarget(glm::vec3 target, float radius);
      /// normalizes given vector
      /// can't be the zero vector
      void setForward(glm::vec3 forward);
      glm::vec3 getTargetForward();
      glm::vec3 getTargetLeft();
      /// how close can the camera get to the top/bottom
      /// of the target. Range is [0, 1].
      float camlimit = 0.9;
  protected:
      void updateView();
      
      float radius = 1.0f;
      glm::vec3 target;
      // for controlling target movement
      glm::vec3 targetForward;
      glm::vec3 targetLeft;
  };
  
} // namesapce end

#endif
