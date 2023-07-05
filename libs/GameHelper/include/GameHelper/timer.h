#ifndef TIMER_H
#define TIMER_H

#include <chrono>

namespace gamehelper {

class Timer {
public:
  Timer() {
    start = std::chrono::high_resolution_clock::now();
    lastUpdate = start;
    currentUpdate = start;
  }
  void Update() {
    lastUpdate = currentUpdate;
    currentUpdate = std::chrono::high_resolution_clock::now();
    frameElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentUpdate - lastUpdate).count();
  }

  long long FrameElapsed() {
      return frameElapsed;
  }

private:

#ifdef _MSC_VER
    using timeDuration = std::chrono::steady_clock::time_point; 
#else
    using timeDuration = std::chrono::time_point<
	std::chrono::_V2::system_clock,
	std::chrono::duration<long int, std::ratio<1, 1000000000>>>;
#endif
  
    timeDuration start;
    timeDuration lastUpdate;
    timeDuration currentUpdate;
    long long frameElapsed = 0;
};

}

#endif
