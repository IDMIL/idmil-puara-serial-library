#include "SerialManager.h"

#include <cstring>
#include <latch>
#include <thread>

void PuaraAPI::SerialManager::scan(std::vector<PuaraAPI::SerialManager::SerialPort>& ports) {
    std::vector<SerialPort> openPorts;
    
    DEBUG_PRINT("Creating port objects");

    for (auto& portInfo : serial::list_ports()) {
        std::string portName = portInfo.port;

#if __APPLE__
        if (portName.find("Bluetooth") != std::string::npos || portName.find("debug") != std::string::npos) {
            continue;
        }
#endif

        DEBUG_PRINT(std::format("Creating port for {}", portName));
        try {
            openPorts.push_back(std::make_shared<serial::Serial>(
                portName,
                BAUD_RATE,
                serial::Timeout::simpleTimeout(READ_WRITE_TIMEOUT_MS)
            ));
            DEBUG_PRINT("Success");
        }
        catch (...) { DEBUG_PRINT("Failure"); }
    }

    DEBUG_PRINT("Done creating port objects");

    std::vector<std::string> portResponses = sendCommandMultiple(openPorts, "ping");

    DEBUG_PRINT("Moving ports that responded to output vector");
    ports.clear();

    for (size_t i = 0; i < portResponses.size(); i++) {
        if (!portResponses[i].empty()) {
            ports.push_back(openPorts[i]);
        }
    }

    DEBUG_PRINT("Done moving; scan complete");
}

std::string PuaraAPI::SerialManager::sendCommand(SerialPort& port, const std::string& command) {
    if (!port->isOpen()) {
        try {
            port->open();
        }
        catch (...) {
            return "";
        }
    }

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

std::vector<std::string> PuaraAPI::SerialManager::sendCommandMultiple(std::vector<SerialPort>& ports, const std::string& command) {
    if (ports.size() == 0) {
        return {};
    }
    
    std::vector<std::string> result(ports.size(), "");
    std::latch l(ports.size());

    auto threadTask = [&](SerialPort& port, const std::string& command, size_t resultIdx){
        std::string response = sendCommand(port, command);
        result[resultIdx] = response;
        l.count_down();
    };

    std::vector<std::thread> workers;

    for (size_t i = 0; i < ports.size(); i++) {
        workers.push_back(std::thread(threadTask, std::ref(ports[i]), command, i));
    }

    l.wait();

    for (auto& worker : workers) {
        worker.join();
    }

    return result;
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