#include "SerialManager.h"

#include <nlohmann/json.hpp>

#include <format>
#include <fstream>
#include <iostream>

using nlohmann::json;

// Forward declares
void invalidMsg();
void modificationMenu(PuaraAPI::SerialManager&, size_t, bool);

int main() {
    PuaraAPI::SerialManager manager;
    std::vector<bool> mask(manager.ports.size(), false);

    std::cout << 
        "Puara Serial Tool: Command-line utility for managing Puara instruments over serial\n"
        "Version 0.1\n" 
        "Created by Ian Doherty, September 2026\n"
    << std::endl;

    while (1) {
        std::string msg = "1. Scan\n";
        int n = 2;

        for (size_t i = 0; i < mask.size(); i++) {
            if (mask[i]) {
                std::string name = manager.sendCommand(manager.ports[i], "whatareyou");

                msg += std::format("{}. Configure \"{}\"\n", std::to_string(n), name);
                msg += std::format("{}. Change settings for \"{}\"\n", std::to_string(n + 1), name);

                n += 2;
            }
        }

        std::cout << msg;

        int choice;
        std::cout << ">> ";
        std::cin >> choice;

        if (choice == 1) {
            manager.scan(mask);
        }
        else if (choice < 0 || choice > n) {
            invalidMsg();
        }
        else {
            int whichActiveDevice = (choice - 2) / 2;
            bool configuring = (choice % 2 == 1);

            // Bluntly converting an "active device index" to a concrete port vector index
            int portIdx;
            for (int i = 0; i < mask.size(); i++) {
                if (mask[i] && whichActiveDevice == 0) {
                    portIdx = i;
                    break;
                }
                else if (mask[i]) {
                    whichActiveDevice--;
                }
            }

            modificationMenu(manager, portIdx, configuring);
        }
    }

    return 0;
}

void invalidMsg() {
    std::cout << "Invalid input, please try again." << std::endl;
}

void modificationMenu(PuaraAPI::SerialManager& manager, size_t portIdx, bool configuration) {
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
    }
    else {
        // Constructs the JSON via user prompts
        // using the existing JSON as a blueprint
        std::string command = (
            configuration ? "readconfig" :
            "readsettings"
        );

        modificationJson = json::parse(manager.sendCommand(manager.ports[portIdx], command));

        for (auto& [key, value] : modificationJson.items()) {
            std::string newValue;
            std::cout << key + ": ";
            std::cin >> newValue;

            value = newValue;
        }
    }

    if (configuration) {
        if (!manager.changeConfig(manager.ports[portIdx], modificationJson)) {
            std::cout << "Config failed, please try again." << std::endl;
        }
        else {
            std::cout << "Config succeeded! Rebooting..." << std::endl;
        }
    }
    else {
        if (!manager.changeSettings(manager.ports[portIdx], modificationJson)) {
            std::cout << "Settings change failed, please try again." << std::endl;
        }
        else {
            std::cout << "Settings change succeeded! Rebooting..." << std::endl;
        }
    }
}