#include <game/camera.h>

#include <GLFW/glfw3.h> // for keyboard button enums
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace camera {

  const glm::vec3 DEFAULT_FORWARD = glm::vec3(1, 0, 0);
  
  Base::Base() { forward = DEFAULT_FORWARD; }
  
  void Base::calcBasis() {
      if(forward == glm::vec3(0))
	  forward = DEFAULT_FORWARD;
      left = glm::normalize(glm::cross(worldUp, forward));
      up = glm::cross(forward, left);
      updateView();
  }
  
  void Base::updateView() {
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

  void Base::setForward(glm::vec3 forward) {
      if(forward != glm::vec3(0))
	  this->forward = glm::normalize(forward);
      else
	  this->forward = DEFAULT_FORWARD;
  }
  

  FirstPerson::FirstPerson() : Base() {
      Base::calcBasis();
  }
  
  FirstPerson::FirstPerson(glm::vec3 position) : Base() {
      this->pos = position;
      Base::calcBasis();
  }

  void FirstPerson::setPos(glm::vec3 pos) {
      this->pos = pos;
  }

  void FirstPerson::control(glm::vec2 ctrlDir) {
      glm::quat yaw(cos(ctrlDir.x), (float)sin(ctrlDir.x) * up);
      glm::quat pitch(cos(ctrlDir.y), (float)sin(ctrlDir.y) * left);
      glm::quat q = pitch * yaw;
      glm::quat c = glm::conjugate(q);
      forward = glm::normalize(q * forward * c);
      Base::calcBasis();
  }

  void FirstPerson::flycamUpdate(Input &input, Timer &timer) {
      float velocity = 0.01f * timer.dt();
      if(input.kb.hold(GLFW_KEY_W))
	  pos -= forward * velocity;
      if(input.kb.hold(GLFW_KEY_A))
	  pos -= left * velocity;
      if(input.kb.hold(GLFW_KEY_S))
	  pos += forward * velocity;
      if(input.kb.hold(GLFW_KEY_D))
	  pos += left * velocity;
      
      if(input.kb.hold(GLFW_KEY_SPACE))
	  pos += worldUp * velocity;
      if(input.kb.hold(GLFW_KEY_LEFT_SHIFT))
	  pos -= worldUp * velocity;
  
      glm::vec2 controller(0);
      if(input.c.connected(0)) {
	  if(input.c.hold(0, GLFW_GAMEPAD_BUTTON_A))
	      pos += worldUp * velocity;
	  if(input.c.hold(0, GLFW_GAMEPAD_BUTTON_B))
	      pos -= worldUp * velocity;
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
      }
      control((-glm::vec2(input.m.dx(), input.m.dy()) + controller)
	      * (float)timer.dt() * 0.00005f);
  }


  /// --- Third Person Camera ---

  ThirdPerson::ThirdPerson() : Base() {
      updateView();
  }

  void ThirdPerson::setTarget(glm::vec3 target, float radius) {
      this->target = target;
      this->radius = radius;
      updateView();
  }

  void ThirdPerson::setForward(glm::vec3 forward) {
      float updot = glm::dot(forward, worldUp);
      if (updot > camlimit || -updot > camlimit)
	  return;
      Base::setForward(forward);
      updateView();
  }

  glm::vec3 ThirdPerson::getTargetForward() { return targetForward; }
  glm::vec3 ThirdPerson::getTargetLeft() { return targetLeft; }

  void ThirdPerson::control(glm::vec2 ctrlDir) {
      if (ctrlDir.x != 0 || ctrlDir.y != 0) {
          float updot = glm::dot(forward, worldUp);
          if (updot > camlimit && ctrlDir.y < 0 ||
              -updot > camlimit && ctrlDir.y > 0)
              ctrlDir.y = 0;
          auto qx = glm::quat(cos(ctrlDir.x), (float)sin(ctrlDir.x) * up);
          auto qy = glm::quat(cos(ctrlDir.y), (float)sin(ctrlDir.y) * left);
          auto q = qx * qy;
          auto c = glm::conjugate(q);
          forward = q * forward * c;
      }
      updateView();
  }

  void ThirdPerson::updateView() {
      pos = this->forward * radius + this->target;
      targetForward = glm::normalize(glm::dot(forward, worldUp) * worldUp - forward);
      targetLeft = glm::normalize(glm::cross(worldUp, getTargetForward()));
      Base::calcBasis();
  }

} // namespace camera
