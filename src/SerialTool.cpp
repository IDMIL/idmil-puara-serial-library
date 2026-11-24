#include <SerialManager.h>

#include <nlohmann/json.hpp>

#include <format>
#include <fstream>
#include <iostream>

using nlohmann::json;

// Forward declares
void invalidMsg();
void configMenu(PuaraAPI::SerialManager::SerialPort&);

std::vector<PuaraAPI::SerialManager::SerialPort> ports;

int main() {
    std::cout << 
        "Puara Serial Tool: Command-line utility for managing Puara instruments over serial\n"
        "Version 0.1\n" 
        "Created by Ian Doherty, September 2026\n"
    << std::endl;

    while (1) {
        std::string msg = "1. Scan\n";
        int n = 2;

        std::vector<std::string> deviceNames = PuaraAPI::SerialManager::sendCommandMultiple(ports, "whatareyou");

        for (auto& name : deviceNames) {
            msg += std::format("{}. Configure \"{}\"\n", std::to_string(n), name);
            n += 1;
        }

        std::cout << msg;

        std::string choiceStr;
        std::cout << ">> ";
        std::cin >> choiceStr;
        int choice;

        try {
            choice = std::stoi(choiceStr);
        }
        catch (...) {
            std::cout << "Please input a number." << std::endl;
            continue;
        }

        if (choice == 1) {
            PuaraAPI::SerialManager::scan(ports);
        }
        else if (choice <= 0 || choice >= n) {
            std::cout << "Invalid choice, please try again." << std::endl;
        }
        else {
            int portIdx = choice - 2;
            configMenu(ports[portIdx]);
        }
    }

    return 0;
}

void configMenu(PuaraAPI::SerialManager::SerialPort& port) {
    std::string msg = "1. Configure via JSON\n2. Configure manually";
    std::cout << msg << std::endl;
    
    json configJson;
    int choice;

    do {
        std::string choiceStr;
        std::cout << ">> ";
        std::cin >> choiceStr;

        try {
            choice = std::stoi(choiceStr);
        }
        catch (...) {
            std::cout << "Please input a number." << std::endl;
            continue;
        }

        if (choice == 1) {
            bool finished = false;

            do {
                std::string path;

                msg = "Enter the path to the JSON config file: ";

                std::cout << msg;
                std::cin >> path;

                std::ifstream f(path);

                try {
                    configJson = json::parse(f);
                    finished = true;
                }
                catch (...) {
                    std::cout << "Invalid JSON, please try again.";
                }
            }
            while (finished == false);
        }
        else if (choice == 2) {
            // Constructs the JSON via user prompts
            // using the existing JSON as a blueprint

            configJson = json::parse(PuaraAPI::SerialManager::sendCommand(port, "readconfig"));

            for (auto& [key, value] : configJson.items()) {
                std::string newValue;
                std::cout << key + ": ";
                std::cin >> newValue;

                if (value.is_number()) {
                    value = std::stoi(newValue);
                }
                else {
                    value = newValue;
                }
            }
        }
        else {
            std::cout << "Invalid choice, please try again." << std::endl;
        }
    }
    while (choice != 1 && choice != 2);

    PuaraAPI::SerialManager::ChangeConfigResult res = PuaraAPI::SerialManager::changeConfig(port, configJson);

    switch (res) {
        case PuaraAPI::SerialManager::ChangeConfigResult::SUCCESS: {
            std::cout << "Config succeeded!" << std::endl;
            break;
        }
        case PuaraAPI::SerialManager::ChangeConfigResult::TIMEOUT: {
            std::cout << "Config failed due to timeout" << std::endl;
            break;
        }
    }

    ports.clear();
}