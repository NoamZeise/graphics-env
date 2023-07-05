#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>
#include "keyboard.h"
#include "mouse.h"

namespace gamehelper {
  struct Input { 
      Keyboard kb;
      Mouse m;

      void update() {
	  kb.update();
	  m.update();
      }
  };
}

#endif
