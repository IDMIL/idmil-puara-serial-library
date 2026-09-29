#include "SerialManager.h"

#include <format>
#include <iostream>
#include <vector>

void main() {
    PuaraAPI::SerialManager manager;
    std::vector<bool> mask(manager.ports.size());

    while (1) {
        manager.scan(mask);

        for (size_t i = 0; i < manager.ports.size(); i++) {
            if (mask[i]) {
                std::string deviceName = manager.sendCommand(manager.ports[i], "whatareyou");
                std::cout << std::format("Found Puara device \"{}\" on port {}", deviceName, manager.ports[i]->getPort()) << std::endl;
            }
        }
    }
}