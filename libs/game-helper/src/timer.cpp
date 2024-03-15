#include <game/timer.h>

using hrc = std::chrono::high_resolution_clock;

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
    if(elapsed < 0) //first update can be negative
	elapsed = 0;
}

long long Timer::dt() {
    return elapsed;
}
