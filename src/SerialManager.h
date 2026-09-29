#pragma once

#include <nlohmann/json.hpp>
#include <serial/serial.h>

#include <format>
#include <iostream>
#include <map>
#include <string>
#include <unordered_set>

using nlohmann::json;

#define DEBUG_MODE 1

static inline void DEBUG_PRINT(const std::string& msg) {
    if (DEBUG_MODE) {
        std::cerr << msg << std::endl;
    }
}

namespace PuaraAPI {

/**
 * Native C++ library for managing Puara serial devices in Arduino-agnostic software
 * @author Ian Doherty
 */
class SerialManager {
public:
    typedef std::unique_ptr<serial::Serial> SerialPort;

    // Constants
    const unsigned BAUD_RATE = 9600;
    const uint32_t READ_WRITE_TIMEOUT_MS = 1000;

    /**
     * Dev-friendly struct representing the possible config settings for a Puara device
     */
    struct ConfigSettings {
        std::string wifiSsid;
        std::string wifiPw;
        std::string destinationIp;
        std::string destinationPort;
    };

    // Constructor
    SerialManager();

    /**
     * Scans for all serial ports connected to Puara devices
     * @param[out] mask A vector representing which ports have detected a Puara device
     * @note Mask must be a predefined vector that is the same size as `ports`
     * @note Puara devices are detected when the reply "pong" to a "ping" command. A Puara-specific
     * serial command with a unique identifier may be more secure/reliable.
     */
    void scan(std::vector<bool>& mask);

    /**
     * Sends a serial command to the Puara device connected at a given port
     * @param port The serial port
     * @param command The command
     */
    std::string sendCommand(SerialPort& port, const std::string& command);
    
    /**
     * Sets a Puara device's config settings
     * @param port The serial port the device is connected to
     * @param configSettings A JSON object representing the desired config settings
     * @returns True if the config was successful, false otherwise
     */
    bool changeConfig(SerialPort& port, const json& configSettings);

    /**
     * Sets a Puara device's settings
     * @param port The serial port the device is connected to
     * @param configSettings A JSON object representing the desired device settings
     * @returns True if the change was successful, false otherwise
     */
    bool changeSettings(SerialPort& port, const json& deviceSettings);

    /**
     * A vector representing all ports on the host device
     */
    std::vector<SerialPort> ports;
};

}