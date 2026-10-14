#include <Timer.h>

#include <exception>
#include <thread>

void Timer::start(std::chrono::duration<long long> duration, std::optional<std::function<void()>> callback) {
    if (isActive()) {
        timerThread = std::thread {}; // kills any existing thread
    }
    
    auto threadTask = [this, duration, callback](){
        this->done = false;
        std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now() + duration;

        std::this_thread::sleep_until(end);

        if (callback) {
            (*callback)();
        }

        this->done = true;
    };

    timerThread = std::thread(threadTask);
}

void Timer::waitUntilDone() {
    if (isActive()) {
        timerThread.join();
    }
}

void Timer::startAndWait(std::chrono::duration<long long> duration, std::optional<std::function<void()>> callback) {
    start(duration, callback);
    waitUntilDone();
}

bool Timer::isActive() {
    return timerThread.joinable();
}

bool Timer::isDone() {
    return done;
}