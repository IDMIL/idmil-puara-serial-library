#include "SerialManager.h"

#include <Timer.h>

#include <cstring>
#include <latch>
#include <thread>
#include <unordered_set>

void PuaraAPI::SerialManager::scan(std::vector<PuaraAPI::SerialManager::SerialPort>& ports) {
    std::vector<SerialPort> openPorts;
    
    for (auto& portInfo : serial::list_ports()) {
        std::string portName = portInfo.port;

#if __APPLE__
        if (portName.find("Bluetooth") != std::string::npos || portName.find("debug") != std::string::npos) {
            continue;
        }
#endif
        try {
            openPorts.push_back(std::make_shared<serial::Serial>(
                portName,
                BAUD_RATE,
                serial::Timeout::simpleTimeout(READ_WRITE_TIMEOUT_MS)
            ));
        }
        catch (...) { /* nothing */ }
    }

    std::vector<std::string> portResponses = sendCommandMultiple(openPorts, "ping");

    ports.clear();

    for (size_t i = 0; i < portResponses.size(); i++) {
        if (!portResponses[i].empty()) {
            ports.push_back(openPorts[i]);
        }
    }
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

    port->flush();
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
        response = port->readline(65536UL, ">>>");
        size_t startIdx = response.find("<<<");
        response = response.substr(startIdx + 3, response.size() - startIdx - 6);
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

void PuaraAPI::SerialManager::changeConfig(PuaraAPI::SerialManager::SerialPort& port, const json& configSettings) {
    std::string currentConfigStr = sendCommand(port, "readconfig");
    DEBUG_PRINT(std::format("Current config: {}", currentConfigStr));

    if (currentConfigStr.empty()) {
        return;
    }

    json configJson = json::parse(currentConfigStr);
    configJson.merge_patch(configSettings);

    DEBUG_PRINT(std::format("New config: {}", configJson.dump()));

    Timer t;
    std::unordered_set<std::string> ignoredKeys; // keys that are too long (command >64 bytes)

    while (configJson.size() > 0) {
        // Attempting to write json line-by-line
        for (auto it = configJson.begin(); it != configJson.end(); it++) {
            std::string command = std::format("sendconfig {}", it->dump());
            DEBUG_PRINT(std::format("Sending \"{}\"", command));

            if (command.size() > 64) {
                DEBUG_PRINT(std::format("Unable to send \"{}\"; command exceeds 64 bytes", command));
                ignoredKeys.insert(it.key());
                continue;
            }

            sendCommand(port, command);
            t.startAndWait(std::chrono::seconds(2), {});
        }

        DEBUG_PRINT("Writing to device memory");
        sendCommand(port, "writeconfig");
        t.startAndWait(std::chrono::seconds(2), {});

        DEBUG_PRINT("Checking results");

        std::string deviceConfigStr = sendCommand(port, "readconfig");
        json deviceConfigJson = json::parse(deviceConfigStr);
        json tmp;

        // Checking for keys that failed to update
        for (auto it = configJson.begin(); it != configJson.end(); it++) {
            if (!ignoredKeys.contains(it.key()) && deviceConfigJson[it.key()] != it.value()) {
                DEBUG_PRINT("Device failed to set a config value!");
                
                DEBUG_PRINT(std::format("Desired: {}", it->dump()));
                DEBUG_PRINT(std::format("Actual (on device): {}", deviceConfigJson[it.key()].dump()));
                
                tmp.push_back(*it);
            }
        }

        configJson = tmp;
    }

    // When we're all done, reboot
    sendCommand(port, "reboot");

    return;
}

void PuaraAPI::SerialManager::changeSettings(SerialPort& port, const json& deviceSettings) {
    std::string currentSettingsStr = sendCommand(port, "readsettings");

    if (currentSettingsStr.empty()) {
        return;
    }

    json settingsJson = json::parse(currentSettingsStr);
    settingsJson.merge_patch(deviceSettings);

    DEBUG_PRINT(settingsJson.dump());

    sendCommand(port, std::format("sendsettings {}", settingsJson.dump()));
    sendCommand(port, "writesettings");
    sendCommand(port, "reboot");
    
    return;
}