#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include "keyboard.h"
#include "mouse.h"
#include "controller.h"

namespace gamehelper {
  struct Input { 
      Keyboard kb;
      Mouse m;
      ControllerManager c;

      void update() {
	  kb.update();
          m.update();
	  c.update();
      }
  };
}

#endif
