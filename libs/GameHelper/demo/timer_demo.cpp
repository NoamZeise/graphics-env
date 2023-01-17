#include "timer_demo.h"

#include <timer.h>

#include <iostream>
#include <thread>
#include <chrono>

void logTimer(gamehelper::Timer &t) {
    std::cout << "> update timer\n";
    t.Update();
    std::cout << "timer.FrameElapsed = " << t.FrameElapsed() << std::endl;
}

void sleepThread(int ms, gamehelper::Timer &t) {
    std::cout << "> sleep thread for " << ms << " milliseconds\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    logTimer(t);
}

void timerDemo() {
    std::cout << "------TIMER DEMO------\n";
    gamehelper::Timer t;
    sleepThread(100, t);
    sleepThread(10, t);
    sleepThread(0, t);
}
