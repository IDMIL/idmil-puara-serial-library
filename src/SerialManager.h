#pragma once

#include <nlohmann/json.hpp>
#include <serial/serial.h>
#include <Timer.h>

#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#define DEBUG_MODE 1

using nlohmann::json;

static inline void DEBUG_PRINT(const std::string& msg) {
#if DEBUG_MODE == 1
    std::cerr << msg << std::endl;
#endif
}

static const unsigned BAUD_RATE = 115200;
static const uint32_t READ_WRITE_TIMEOUT_MS = 1000;
static const std::chrono::seconds COMMAND_COOLDOWN {3};

static Timer commandCooldownTimer;

namespace PuaraAPI {

/**
 * C++ static library for managing Puara devices over serial
 * @author Ian Doherty
 */
class SerialManager {
public:
    typedef std::shared_ptr<serial::Serial> SerialPort;

    SerialManager() = delete;
    
    /**
     * Scans for all serial ports connected to Puara devices
     * @returns A vector of SerialPort objects
     * @note Mask must be a predefined vector that is the same size as `ports`
     * @note Puara devices are detected when the reply "pong" to a "ping" command. A Puara-specific
     * serial command with a unique identifier may be more secure/reliable.
     */
    static void scan(std::vector<SerialPort>& ports);

    /**
     * Sends a serial command to the Puara device connected at a given port
     * @param port The serial port
     * @param command The command
     * @param useCooldown Start a cooldown timer after the command is sent. If running in parallel,
     * this should be set to false.
     * @returns The Puara-defined response, or "ERROR" on an unexpected response
     */
    static std::string sendCommand(SerialPort& port, const std::string& command, bool useCooldown = true);
    
    /**
     * Sends a serial command to multiple Puara devices connected in parallel
     * @param ports A vector of serial ports
     * @param command The command
     */
    static std::vector<std::string> sendCommandMultiple(std::vector<SerialPort>& ports, const std::string& command);

    struct ChangeConfigProgress {
        size_t numFieldsChanged = 0;
        size_t numFieldsTotal = 0;

        // Synchronization & notification
        std::mutex mtx;
        std::condition_variable cv;
    };

    /**
     * Sets a Puara device's config settings
     * @param port The serial port the device is connected to
     * @param newConfig A JSON object representing the desired config settings
     * @param[out] progress An object representing the progress of the config operation
     */
    static void changeConfig(SerialPort& port, const json& newConfig, ChangeConfigProgress* progress = nullptr);
};

}