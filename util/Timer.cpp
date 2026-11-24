#include <Timer.h>

#include <exception>
#include <thread>

Timer::~Timer() {
    if (timerThread.joinable()) {
        stop(true);
    }
}

void Timer::start(std::chrono::duration<long long> duration, std::optional<std::function<void()>> callback) {
    done = false;
    overrideCallback = false;

    auto timerThreadTask = [this, duration, callback](){
        std::unique_lock lk {mtx};

        cv.wait_for(lk, duration, [this](){ return done; });
        lk.unlock();

        if (callback && !overrideCallback) {
            (*callback)();
        }
    };

    timerThread = std::thread(timerThreadTask);
}

void Timer::stop(bool overrideCallback) {
    this->overrideCallback = overrideCallback;

    {
        std::lock_guard<std::mutex> lk(mtx);
        done = true;
    }
    
    cv.notify_one();
    timerThread.join();
}

void Timer::startAndWait(std::chrono::duration<long long> duration, std::optional<std::function<void()>> callback) {
    start(duration, callback);
    timerThread.join();
}

bool Timer::isActive() {
    return !done;
}