#include <game/camera.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace camera {
  FirstPerson::FirstPerson(glm::vec3 position) : Base() {
      this->pos = position;
      calculateVectors();
  }

  glm::mat4 FirstPerson::getView() {
      return view;
  }

  glm::vec3 FirstPerson::getPos() { return this->pos; }
  void FirstPerson::setPos(glm::vec3 pos) {
      this->pos = pos;
  }

  void FirstPerson::control(glm::vec2 ctrlDir) {
      yaw += ctrlDir.x * inputSensitivity;
      pitch += ctrlDir.y * inputSensitivity;
      if(pitch > 89.0f)
	  pitch = 89.0f;
      if(pitch < -89.0f)
	  pitch = -89.0f;
      calculateVectors();
  }

  void FirstPerson::flycamUpdate(Input &input, Timer &timer) {
      float velocity = 0.01f * timer.dt();
      if(input.kb.hold(GLFW_KEY_W))
	  pos += forward * velocity;
      if(input.kb.hold(GLFW_KEY_A))
	  pos += left * velocity;
      if(input.kb.hold(GLFW_KEY_S))
	  pos -= forward * velocity;
      if(input.kb.hold(GLFW_KEY_D))
	  pos -= left * velocity;
      
      if(input.kb.hold(GLFW_KEY_SPACE))
	  pos += worldUp * velocity;
      if(input.kb.hold(GLFW_KEY_LEFT_SHIFT))
	  pos -= worldUp * velocity;

      
      
      glm::vec2 controller(0);
      if(input.c.connected(0)) {
	  // Left Stick - camera view
	  controller = glm::vec2(input.c.axis(0, GLFW_GAMEPAD_AXIS_LEFT_X),
			       input.c.axis(0, GLFW_GAMEPAD_AXIS_LEFT_Y));
	  
	  controller.x = fabs(controller.x) > 0.15 ? controller.x : 0;
	  controller.y = fabs(controller.y) > 0.15 ? controller.y : 0;
	  pos += forward * velocity * -controller.y;
	  pos -= left * velocity * controller.x;

	  // Right Stick - camera rotation
	  controller = glm::vec2(input.c.axis(0, GLFW_GAMEPAD_AXIS_RIGHT_X),
				 input.c.axis(0, GLFW_GAMEPAD_AXIS_RIGHT_Y));
	  
	  controller.x = fabs(controller.x) > 0.15 ? controller.x : 0;
	  controller.y = fabs(controller.y) > 0.15 ? controller.y : 0;

	  if(input.c.hold(0, GLFW_GAMEPAD_BUTTON_A))
	      pos += worldUp * velocity;
	  if(input.c.hold(0, GLFW_GAMEPAD_BUTTON_B))
	      pos -= worldUp * velocity;
      }
      control(-glm::vec2(input.m.dx(), input.m.dy()) * 0.1f +
	      -controller);
  }

  void FirstPerson::calculateVectors() {
      forward.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
      forward.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
      forward.z = sin(glm::radians(pitch));
      forward = glm::normalize(forward);
      left = glm::normalize(glm::cross(worldUp, forward));
      up = glm::normalize(glm::cross(forward, left));
      
      view = glm::lookAt(pos, pos + forward, up);
  }


  /// --- Third Person Camera ---

  ThirdPerson::ThirdPerson() : Base() {
      pos = glm::vec3(1, 0, 0);
      worldPos = pos;
      updateView();
  }

  void ThirdPerson::setTarget(glm::vec3 target, float radius) {
      this->target = target;
      this->radius = radius;
      targetMat = glm::translate(glm::mat4(1.0f), -target);
      this->worldPos = target + (pos * radius);
      updateView();
  }

  glm::vec3 ThirdPerson::getPos() { return worldPos; }
  void ThirdPerson::setPos(glm::vec3 pos) {
      if(pos != glm::vec3(0))
	  this->pos = glm::normalize(pos);
      else
	  this->pos = pos;
  }
  
  glm::mat4 ThirdPerson::getView() { return view; }

  glm::vec3 ThirdPerson::getWorldUp() { return worldUp; }
  void ThirdPerson::setWorldUp(glm::vec3 worldUp) {
      if(worldUp != glm::vec3(0))
	  this->worldUp = glm::normalize(worldUp);
  }

  glm::vec3 ThirdPerson::getTargetForward() { return targetForward; }
  glm::vec3 ThirdPerson::getTargetLeft() { return targetLeft; }

  void ThirdPerson::control(glm::vec2 ctrlDir) {
      ctrlDir *= inputSensitivity;
      if (ctrlDir.x != 0 || ctrlDir.y != 0) {
          float updot = glm::dot(forward, worldUp);
          if (updot > camlimit && ctrlDir.y < 0 ||
              -updot > camlimit && ctrlDir.y > 0)
              ctrlDir.y = 0;
          auto qx = glm::quat(cos(ctrlDir.x), (float)sin(ctrlDir.x) * up);
          auto qy = glm::quat(cos(ctrlDir.y), (float)sin(ctrlDir.y) * left);
          auto q = qx * qy;
          auto c = glm::conjugate(q);
          pos = q * pos * c;
      }
      updateView();
  }

  void ThirdPerson::updateView() {
      targetForward = glm::normalize(glm::dot(pos, worldUp) * worldUp - pos);
      targetLeft = glm::normalize(glm::cross(worldUp, targetForward));
      forward = pos;
      glm::vec3 pos = this->pos * radius + this->target;
      // we normalize here as worldUp and forward
      // are not perpendicular in general
      left = glm::normalize(glm::cross(worldUp, forward));
      up = glm::cross(forward, left);
      view = glm::mat4(1.0f);
      view[0][0] = left.x;
      view[1][0] = left.y;
      view[2][0] = left.z;
      view[3][0] = -glm::dot(left, pos);
      view[0][1] = up.x;
      view[1][1] = up.y;
      view[2][1] = up.z;
      view[3][1] = -glm::dot(up, pos);
      view[0][2] = forward.x;
      view[1][2] = forward.y;
      view[2][2] = forward.z;
      view[3][2] = -glm::dot(forward, pos);
  }

} // namespace camera
