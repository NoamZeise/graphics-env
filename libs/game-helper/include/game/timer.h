#ifndef TIMER_H
#define TIMER_H

#include <chrono>

class Timer {
public:
    Timer();
    // call each frame to use dt() as frame time
    void update();
    // return time since last update in milliseconds
    long long dt();
private:

#ifdef _MSC_VER
    using timeDuration = std::chrono::steady_clock::time_point; 
#else
    using timeDuration = std::chrono::time_point<
	std::chrono::_V2::system_clock,
	std::chrono::duration<long int, std::ratio<1, 1000000000>>>;
#endif
    timeDuration lastUpdate;
    timeDuration currentUpdate;
    long long elapsed = 0;
};

#endif
