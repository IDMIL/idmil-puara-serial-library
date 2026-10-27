#include "SerialManager.h"

#include <Timer.h>

#include <cstring>
#include <latch>
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

std::string PuaraAPI::SerialManager::sendCommand(SerialPort& port, const std::string& command, bool useCooldown) {
    if (!port->isOpen()) {
        try {
            port->open();
        }
        catch (...) {
            return "ERROR";
        }
    }

    if (useCooldown && !commandCooldownTimer.isDone()) {
        commandCooldownTimer.waitUntilDone();
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

        if (startIdx == std::string::npos) {
            response = "ERROR";
        }
        else {
            response = response.substr(startIdx + 3, response.size() - startIdx - 6);
        }
    }

    if (useCooldown) {
        commandCooldownTimer.start(COMMAND_COOLDOWN, {});
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
        std::string response = sendCommand(port, command, false);
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

void PuaraAPI::SerialManager::changeConfig(PuaraAPI::SerialManager::SerialPort& port, const json& newConfig, ChangeConfigProgress* progress) {
    std::string currentConfigStr = "";
    do {
        currentConfigStr = sendCommand(port, "readconfig");
    }
    while (currentConfigStr == "ERROR");
    
    json currentConfigJson = json::parse(currentConfigStr);

    DEBUG_PRINT(std::format("Current config: {}", currentConfigJson.dump()));
    DEBUG_PRINT(std::format("New config: {}", newConfig.dump()));

    json finalConfig {};
    size_t numSkipped = 0;

    // Isolating config values that need to change
    for (auto it = newConfig.begin(); it != newConfig.end(); it++) {
        if (currentConfigJson[it.key()] == it.value()) {
            DEBUG_PRINT(std::format("Key \"{}\" already set to {}, skipping", it.key(), it.value()));
            numSkipped++;
        }
        else {
            finalConfig[it.key()] = it.value();
        }
    }

    DEBUG_PRINT(std::format("Skipped {}/{} keys", numSkipped, newConfig.size()));
    DEBUG_PRINT(std::format("Final config: {}", finalConfig.dump()));

    if (progress != nullptr) {
        progress->mtx.lock();
        progress->numFieldsTotal = finalConfig.size();
        progress->mtx.unlock();
        progress->cv.notify_all();
    }
    
    std::unordered_set<std::string> ignoredKeys; // keys that are too long (command >64 bytes)

    while (finalConfig.size() > 0) {
        // Attempting to write json line-by-line
        for (auto it = finalConfig.begin(); it != finalConfig.end(); it++) {
            std::string command = std::format("sendconfig {{\"{}\": {}}}", it.key(), it.value());
            DEBUG_PRINT(std::format("Sending \"{}\"", command));

            if (command.size() > 64) {
                DEBUG_PRINT(std::format("Unable to send \"{}\"; command exceeds 64 bytes", command));
                ignoredKeys.insert(it.key());
                continue;
            }

            sendCommand(port, command);
        }

        sendCommand(port, "writeconfig");
        DEBUG_PRINT("Checking results");

        do {
            currentConfigStr = sendCommand(port, "readconfig");
        }
        while (currentConfigStr == "ERROR");
        
        currentConfigJson = json::parse(currentConfigStr);
        json failed {};

        size_t numFailed = 0;

        // Checking for keys that failed to update
        for (auto it = finalConfig.begin(); it != finalConfig.end(); it++) {
            if (!ignoredKeys.contains(it.key()) && currentConfigJson[it.key()] != it.value()) {
                DEBUG_PRINT("Device failed to set a config value!");
                
                DEBUG_PRINT(std::format("Desired: {{\"{}\": {}}}", it.key(), it.value()));
                DEBUG_PRINT(std::format("Actual (on device): {{\"{}\": {}}}", it.key(), currentConfigJson[it.key()].dump()));
                
                failed[it.key()] = it.value();
                numFailed++;
            }
        }

        if (numFailed > 0) {
            DEBUG_PRINT(std::format("{}/{} keys failed to update ({}%)", numFailed, finalConfig.size(), (int) (100 * ((float) numFailed / (float) finalConfig.size()))));
        }

        finalConfig = failed;

        if (progress != nullptr) {
            progress->mtx.lock();
            progress->numFieldsChanged = progress->numFieldsTotal - finalConfig.size();
            progress->mtx.unlock();
            progress->cv.notify_all();
        }
    }

    // When we're all done, write & reboot
    DEBUG_PRINT("Done configuring, rebooting...");
    sendCommand(port, "reboot");

    return;
}