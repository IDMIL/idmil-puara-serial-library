#include "SerialManager.h"

#include <GetComPortList.h>

#include <cstring>

PuaraAPI::SerialManager::SerialManager() {
    for (const std::string& portName : GetComPortList::get_list_serial_ports()) {
        SerialPort port = std::make_unique<serial::Serial>(
            portName, 
            BAUD_RATE, 
            serial::Timeout::simpleTimeout(READ_WRITE_TIMEOUT_MS)
        );
        ports.push_back(std::move(port));
    }
}

void PuaraAPI::SerialManager::scan(std::vector<bool>& mask) {
    assert(mask.size() == ports.size());

    for (size_t i = 0; i < ports.size(); i++) {
        SerialPort& port = ports[i];

        // If the port isn't open, try opening it first
        if (!port->isOpen()) {
            try {
                port->open();
            }
            catch (...) {
                mask[i] = false;
            }
        }

        // Try sending a ping
        if (port->isOpen()) {
            std::string response = sendCommand(port, "ping");
            mask[i] = !response.empty();
        }
    }
}

// TODO: Error handling
std::string PuaraAPI::SerialManager::sendCommand(SerialPort& port, const std::string& command) {
    port->write(command);
    std::string response = "";

    if (command == "reset" || command == "reboot") {
        port->read(1); // eating the prefixed newline
        response = port->readline();
    }
    else if (command == "ping") {
        response = port->readline();
    }
    else if (command == "whatareyou" || command == "readconfig" || command == "readsettings") {
        response = port->readline(65536UL, ">>>").substr(3);
    }

    return response;
}

bool PuaraAPI::SerialManager::changeConfig(PuaraAPI::SerialManager::SerialPort& port, const json& configSettings) {
    std::string currentConfigStr = sendCommand(port, "readconfig");

    if (currentConfigStr.empty()) {
        return false;
    }

    json configJson = json::parse(currentConfigStr);
    configJson.merge_patch(configSettings);

    DEBUG_PRINT(configJson.dump());

    sendCommand(port, std::format("sendconfig {}", configJson.dump()));
    sendCommand(port, "writeconfig");

    std::string rebootResponse = sendCommand(port, "reboot");

    return !rebootResponse.empty();
}

bool PuaraAPI::SerialManager::changeSettings(SerialPort& port, const json& deviceSettings) {
    std::string currentSettingsStr = sendCommand(port, "readsettings");

    if (currentSettingsStr.empty()) {
        return false;
    }

    json settingsJson = json::parse(currentSettingsStr);
    settingsJson.merge_patch(deviceSettings);

    DEBUG_PRINT(settingsJson.dump());

    sendCommand(port, std::format("sendsettings {}", settingsJson.dump()));
    sendCommand(port, "writesettings");

    std::string rebootResponse = sendCommand(port, "reboot");

    return !rebootResponse.empty();
}