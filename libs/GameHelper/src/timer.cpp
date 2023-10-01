#include <GameHelper/timer.h>

namespace gamehelper {
  Timer::Timer() {
    start = std::chrono::high_resolution_clock::now();
    lastUpdate = start;
    currentUpdate = start;
  }

  void Timer::Update() {
      lastUpdate = currentUpdate;
      currentUpdate = std::chrono::high_resolution_clock::now();
      frameElapsed = std::chrono::duration_cast<std::chrono::milliseconds>
	  (currentUpdate - lastUpdate).count();
  }

  long long Timer::FrameElapsed() {
      return frameElapsed;
  }
}
