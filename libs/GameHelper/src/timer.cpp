#include <GameHelper/timer.h>

using hrc = std::chrono::high_resolution_clock;

namespace gamehelper {
  Timer::Timer() {
      timeDuration start = hrc::now();
      lastUpdate = start;
      currentUpdate = start;
  }

  void Timer::update() {
      lastUpdate = currentUpdate;
      currentUpdate = hrc::now();
      elapsed = std::chrono::duration_cast<std::chrono::milliseconds>
	  (currentUpdate - lastUpdate).count();
  }

  long long Timer::dt() {
      return elapsed;
  }
}
