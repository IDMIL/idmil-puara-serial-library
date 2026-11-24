#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>

/**
 * Timer: A simple C++11 timer implementation
 * @author Ian Doherty
 */
class Timer {
public:
    ~Timer();

    /**
     * Starts the timer
     * @param duration The duration of the timer
     * @param callback An optional callback function that runs once the timer finishes
     */
    void start(std::chrono::duration<long long> duration, std::optional<std::function<void()>> callback);

    /**
     * Stops the timer
     * @param overrideCallback Optional flag to override the callback this timer was set to use
     */
    void stop(bool overrideCallback = false);

    /**
     * Starts the timer and waits for it to finish
     * @param duration The duration of the timer
     * @param callback An optional callback function that runs once the timer finishes (BEFORE it relinquishes control to the calling thread)
     */
    void startAndWait(std::chrono::duration<long long> duration, std::optional<std::function<void()>> callback);

    /**
     * Returns true if the timer is active, false otherwise
     */
    bool isActive();

private:
    bool done = false;
    std::atomic<bool> overrideCallback = false;
    
    std::condition_variable cv;
    std::mutex mtx;

    std::thread timerThread;
};