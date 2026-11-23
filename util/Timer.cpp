#include <Timer.h>

#include <exception>
#include <thread>

void Timer::start(std::chrono::duration<long long> duration, std::optional<std::function<void()>> callback) {
    if (isActive()) {
        timerThread.join();
    }
    
    auto threadTask = [&](){
        std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now() + duration;
        std::this_thread::sleep_until(end);

        if (callback) {
            (*callback)();
        }
    };

    timerThread = std::thread(threadTask);
}

void Timer::startAndWait(std::chrono::duration<long long> duration, std::optional<std::function<void()>> callback) {
    start(duration, callback);
    timerThread.join();
}

bool Timer::isActive() {
    return timerThread.joinable();
}