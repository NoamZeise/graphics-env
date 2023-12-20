#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>

#include "input.h"
#include "timer.h"

namespace camera {
  
  class FirstPerson {
  public:
      FirstPerson() { _position = glm::vec3(0.0f, 0.0f, 0.0f); };
      FirstPerson(glm::vec3 position);
      glm::mat4 getViewMatrix();
      float getZoom();
      glm::vec3 getPos() { return _position; }
      void setPos(glm::vec3 pos);
      virtual void update(Input &input, Timer &timer);
  protected:
      glm::vec3 _position;
      glm::vec3 _front;
      glm::vec3 _up;
      glm::vec3 _right;
      glm::vec3 _worldUp = glm::vec3(0.0f, 0.0f, 1.0f);
      glm::mat4 view = glm::mat4(1.0f);
      bool viewUpdated = true;

      float _yaw = 200.0f;
      float _pitch = -20.0f;

      float _speed = 0.01f;
      float _sensitivity = 0.05f;
      float _zoom = 45.0f;

      void calculateVectors();
  };

  class ThirdPerson {
  public:
      ThirdPerson();
      void control(glm::vec2 ctrlDir);
      void setTarget(glm::vec3 target, float radius);
      glm::mat4 getView() { return view; }
      
      glm::vec3 getPos() { return worldPos; }
      void setPos(glm::vec3 pos);
      glm::vec3 getLocalPos() { return pos; }
      void setWorldUp(glm::vec3 worldUp);
      glm::vec3 getWorldUp() { return worldUp; }
      glm::vec3 getTargetForward() { return targetForward; }
      glm::vec3 getTargetLeft() { return targetLeft; }

      float inputSensitivity = 1.0f;
      /// how close can the camera get to the top/bottom
      /// of the target. Range is [0, 1].
      float camlimit = 0.9; 
      
  private:
      void updateView();
    
      glm::vec3 worldUp = glm::normalize(glm::vec3(0, 0, 1));

      //local space props
      glm::vec3 pos;
      glm::vec3 forward, up, left;

      // target/world space stuff
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
