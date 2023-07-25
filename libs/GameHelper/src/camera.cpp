#include <GameHelper/camera.h>

#include <GLFW/glfw3.h>

namespace camera {
  FirstPerson::FirstPerson(glm::vec3 position) {
      _position = position;
      calculateVectors();
  }

  glm::mat4 FirstPerson::getViewMatrix() {
      if(viewUpdated) {
	  view = glm::lookAt(_position, _position + _front, _up);
	  viewUpdated = false;
	  return view;
      }
      return view;
  }

  float FirstPerson::getZoom() {
      return _zoom;
  }


  void FirstPerson::update(gamehelper::Input &input, gamehelper::Timer &timer) {
      viewUpdated = true;
      //keyboard
      float velocity = _speed * timer.FrameElapsed();
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

      //mouse
      _pitch   -= (float)input.m.dy() * _sensitivity;
      _yaw 	 -= (float)input.m.dx() * _sensitivity;

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

      //scroll
      _zoom -= (float)input.m.scroll() * timer.FrameElapsed();
      if(_zoom < 1.0f)
	  _zoom = 1.0f;
      if(_zoom > 100.0f)
	  _zoom = 100.0f;

      calculateVectors();
      //std::cout << "X:" << _position.x << " Y:" << _position.y << " Z:" << _position.z << std::endl;
  }

  void FirstPerson::calculateVectors() {
      _front.x = cos(glm::radians(_yaw)) * cos(glm::radians(_pitch));
      _front.y = sin(glm::radians(_yaw)) * cos(glm::radians(_pitch));
      _front.z = sin(glm::radians(_pitch));
      _front = glm::normalize(_front);

      _right = glm::normalize(glm::cross(_front, _worldUp));
      _up = glm::normalize(glm::cross(_right, _front));
  }

} //namespace end
