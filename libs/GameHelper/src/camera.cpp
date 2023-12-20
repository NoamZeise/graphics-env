#include <game/camera.h>

#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace camera {
  FirstPerson::FirstPerson(glm::vec3 position) {
      _position = position;
      calculateVectors();
  }

  glm::mat4 FirstPerson::getViewMatrix() {
      if(viewUpdated) {
	  view = glm::lookAt(_position, _position + _front, _up);
	  viewUpdated = false;
      }
      return view;
  }

  float FirstPerson::getZoom() {
      return glm::radians(_zoom);
  }

  void FirstPerson::setPos(glm::vec3 pos) {
      _position = pos;
  }

  void FirstPerson::update(Input &input, Timer &timer) {
      viewUpdated = true;

      float velocity = _speed * timer.dt();
      if(input.kb.hold(GLFW_KEY_W))
	  _position += _front * velocity;
      if(input.kb.hold(GLFW_KEY_A))
	  _position -= _right * velocity;
      if(input.kb.hold(GLFW_KEY_S))
	  _position -= _front * velocity;
      if(input.kb.hold(GLFW_KEY_D))
	  _position += _right * velocity;
      
      if(input.kb.hold(GLFW_KEY_SPACE))
	  _position += _worldUp * velocity;
      if(input.kb.hold(GLFW_KEY_LEFT_SHIFT))
	  _position -= _worldUp * velocity;

      _pitch -= (float)input.m.dy() * _sensitivity;
      _yaw -= (float)input.m.dx() * _sensitivity;

      if(input.c.connected(0)) {
	  glm::vec2 controller(input.c.axis(0, GLFW_GAMEPAD_AXIS_LEFT_X),
			       input.c.axis(0, GLFW_GAMEPAD_AXIS_LEFT_Y));
	  
	  controller.x = abs(controller.x) > 0.15 ? controller.x : 0;
	  controller.y = abs(controller.y) > 0.15 ? controller.y : 0;
	  _position += _front * velocity * -controller.y;
	  _position += _right * velocity * controller.x;
	  
	  controller = glm::vec2(input.c.axis(0, GLFW_GAMEPAD_AXIS_RIGHT_X),
				 input.c.axis(0, GLFW_GAMEPAD_AXIS_RIGHT_Y));
	  
	  controller.x = abs(controller.x) > 0.15 ? controller.x : 0;
	  controller.y = abs(controller.y) > 0.15 ? controller.y : 0;

	  if(input.c.hold(0, GLFW_GAMEPAD_BUTTON_A))
	      _position += _worldUp * velocity;
	  if(input.c.hold(0, GLFW_GAMEPAD_BUTTON_B))
	      _position -= _worldUp * velocity;
	  
	  _pitch -= controller.y;
	  _yaw -= controller.x;

      }

      if(_pitch > 89.0f)
	  _pitch = 89.0f;
      if(_pitch < -89.0f)
	  _pitch = -89.0f;

      _zoom -= (float)input.m.scroll() * timer.dt();
      if(_zoom < 1.0f)
	  _zoom = 1.0f;
      if(_zoom > 100.0f)
	  _zoom = 100.0f;

      calculateVectors();
  }

  void FirstPerson::calculateVectors() {
      _front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
      _front.y = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
      _front.z = sin(glm::radians(_pitch));
      _front = glm::normalize(_front);

      _right = glm::normalize(glm::cross(_front, _worldUp));
      _up = glm::normalize(glm::cross(_right, _front));
  }



  /// --- Third Person Camera ---

  ThirdPerson::ThirdPerson() {
      pos = glm::vec3(1, 0, 0);
      worldPos = pos;
      updateView();
  }

  void ThirdPerson::setWorldUp(glm::vec3 worldUp) {
      this->worldUp = glm::normalize(worldUp);
      updateView();
  }

  void ThirdPerson::setTarget(glm::vec3 target, float radius) {
      this->target = target;
      this->radius = radius;
      targetMat = glm::translate(glm::mat4(1.0f), -target);
      this->worldPos = target + (pos * radius);
      updateView();
  }

  void ThirdPerson::setPos(glm::vec3 pos) { this->pos = pos; }

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
