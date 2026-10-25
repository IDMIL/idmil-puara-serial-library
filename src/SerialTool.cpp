#include <SerialManager.h>

#include <nlohmann/json.hpp>

#include <csignal>
#include <format>
#include <fstream>
#include <iostream>

using nlohmann::json;

// Forward declares
void invalidMsg();
void configMenu(PuaraAPI::SerialManager::SerialPort&);

// SIGINT
void handler(int) {
    exit(0);
}

int main() {
    std::signal(SIGINT, handler);

    std::cout << 
        "Puara Serial Tool: Command-line utility for managing Puara instruments over serial\n"
        "Version 0.1\n" 
        "Created by Ian Doherty, September 2026\n"
    << std::endl;

    std::vector<PuaraAPI::SerialManager::SerialPort> ports;

    while (1) {
        std::string msg = "1. Scan\n";
        int n = 2;

        std::vector<std::string> deviceNames = PuaraAPI::SerialManager::sendCommandMultiple(ports, "whatareyou");

        for (auto& name : deviceNames) {
            msg += std::format("{}. Configure \"{}\"\n", std::to_string(n), name);
            n += 1;
        }

        std::cout << msg;

        int choice;
        std::cout << ">> ";
        std::cin >> choice;

        if (choice == 1) {
            PuaraAPI::SerialManager::scan(ports);
        }
        else if (choice < 0 || choice > n) {
            invalidMsg();
        }
        else {
            int portIdx = choice - 2;
            configMenu(ports[portIdx]);
        }
    }

    return 0;
}

void invalidMsg() {
    std::cout << "Invalid input, please try again." << std::endl;
}

void configMenu(PuaraAPI::SerialManager::SerialPort& port) {
    std::string msg = "Configure via JSON\n2. Configure manually";
    std::cout << msg << std::endl;
    
    int choice;
    std::cout << ">> ";
    std::cin >> choice;

    json configJson;

    if (choice == 1) {
        std::string path;

        msg = "Enter the path to the JSON config file: ";

        std::cout << msg;
        std::cin >> path;

        std::ifstream f(path);

        configJson = json::parse(f);
    }
    else {
        // Constructs the JSON via user prompts
        // using the existing JSON as a blueprint

        configJson = json::parse(PuaraAPI::SerialManager::sendCommand(port, "readconfig"));

        for (auto& [key, value] : configJson.items()) {
            std::string newValue;
            std::cout << key + ": ";
            std::cin >> newValue;

            value = newValue;
        }
    }

    PuaraAPI::SerialManager::changeConfig(port, configJson);
    std::cout << "Config succeeded!" << std::endl;
}