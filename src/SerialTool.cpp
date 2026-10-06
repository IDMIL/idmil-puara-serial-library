#include <SerialManager.h>

#include <nlohmann/json.hpp>

#include <csignal>
#include <format>
#include <fstream>
#include <iostream>

using nlohmann::json;

// Forward declares
void invalidMsg();
void modificationMenu(PuaraAPI::SerialManager::SerialPort&, bool);

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
            msg += std::format("{}. Change settings for \"{}\"\n", std::to_string(n + 1), name);
            n += 2;
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
            int portIdx = (choice - 2) / 2;
            bool configuring = (choice % 2 == 0);
            modificationMenu(ports[portIdx], configuring);
        }
    }

    return 0;
}

void invalidMsg() {
    std::cout << "Invalid input, please try again." << std::endl;
}

void modificationMenu(PuaraAPI::SerialManager::SerialPort& port, bool configuration) {
    std::string msg = (
        configuration ? "1. Configure via JSON\n2. Configure manually" :
        "1. Change settings via JSON\n2. Change settings manually"
    );  
    std::cout << msg << std::endl;
    
    int choice;
    std::cout << ">> ";
    std::cin >> choice;

    json modificationJson;

    if (choice == 1) {
        std::string path;

        msg = (
            configuration ? "Enter the path to the JSON config file: " :
            "Enter the path to the JSON settings file: "
        );

        std::cout << msg;
        std::cin >> path;

        std::ifstream f(path);

        modificationJson = json::parse(f);

        // try {
            
        // }
        // catch (...) {
        //     std::cout << "Could not parse JSON. Please try again.";
        //     return;
        // }
    }
    else {
        // Constructs the JSON via user prompts
        // using the existing JSON as a blueprint
        std::string command = (
            configuration ? "readconfig" :
            "readsettings"
        );

        modificationJson = json::parse(PuaraAPI::SerialManager::sendCommand(port, command));

        for (auto& [key, value] : modificationJson.items()) {
            std::string newValue;
            std::cout << key + ": ";
            std::cin >> newValue;

            value = newValue;
        }
    }

    if (configuration) {
        if (!PuaraAPI::SerialManager::changeConfig(port, modificationJson)) {
            std::cout << "Config failed, please try again." << std::endl;
        }
        else {
            std::cout << "Config succeeded!" << std::endl;
        }
    }
    else {
        if (!PuaraAPI::SerialManager::changeSettings(port, modificationJson)) {
            std::cout << "Settings change failed, please try again." << std::endl;
        }
        else {
            std::cout << "Settings change succeeded!" << std::endl;
        }
    }
}