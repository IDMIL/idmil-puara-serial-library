#pragma once

#include <nlohmann/json.hpp>
#include <serial/serial.h>

#include <format>
#include <iostream>
#include <memory>
#include <string>

#define DEBUG_MODE 1

using nlohmann::json;

static inline void DEBUG_PRINT(const std::string& msg) {
#if DEBUG_MODE == 1
    std::cerr << msg << std::endl;
#endif
}

static const unsigned BAUD_RATE = 115200;
static const uint32_t READ_WRITE_TIMEOUT_MS = 1000;
static const uint32_t CONFIGURE_REBOOT_WAIT_MS = 3000;

namespace PuaraAPI {

/**
 * Native C++ static library for managing Puara serial devices in non-Arduino software
 * @author Ian Doherty
 */
class SerialManager {
public:
    typedef std::shared_ptr<serial::Serial> SerialPort;
    
    const uint32_t sendCommandCooldownMS = 1000;

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
     */
    static std::string sendCommand(SerialPort& port, const std::string& command);
    
    /**
     * Sends a serial command to multiple Puara devices connected in parallel
     * @param ports A vector of serial ports
     * @param command The command
     */
    static std::vector<std::string> sendCommandMultiple(std::vector<SerialPort>& ports, const std::string& command);

    /**
     * Sets a Puara device's config settings
     * @param port The serial port the device is connected to
     * @param configSettings A JSON object representing the desired config settings
     * @returns True if the config was successful, false otherwise
     */
    static bool changeConfig(SerialPort& port, const json& configSettings);

    /**
     * Sets a Puara device's settings
     * @param port The serial port the device is connected to
     * @param configSettings A JSON object representing the desired device settings
     * @returns True if the change was successful, false otherwise
     */
    static bool changeSettings(SerialPort& port, const json& deviceSettings);

private:
    static std::vector<std::string> getPortAddresses();
};

}