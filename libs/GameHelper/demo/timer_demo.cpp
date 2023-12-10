#include <game/timer.h>

#include <iostream>
#include <thread>
#include <chrono>

void logTimer(Timer &t) {
    std::cout << "> update timer\n";
    t.update();
    std::cout << "timer.FrameElapsed = " << t.dt() << std::endl;
}

void sleepThread(int ms, Timer &t) {
    std::cout << "> sleep thread for " << ms << " milliseconds\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    logTimer(t);
}

int main() {
    std::cout << "------TIMER DEMO------\n";
    Timer t;
    sleepThread(100, t);
    sleepThread(10, t);
    sleepThread(0, t);
}
