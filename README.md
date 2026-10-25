# puara-serial-library: Native C++ library for managing Puara serial devices in parallel

## Description

`idmil-puara-serial-library` is a basic C++ serial library designed to allow for abstracted serial communication between a host machine running this software and a device running Puara firmware.

Parts of this library implicitly target Puara version 0.3 (particularly `SerialManager::changeConfig` in `src/SerialManager.cpp`) as this is the version used by the [T-Stick](https://github.com/IDMIL/T-Stick), the DMI this library was developed for. Future versions of Puara will address certain serial bugs, which will necessitate an update of this both this library *and* the T-Stick firmware. See the targeted changes here: [issue1](https://github.com/Puara/puara-module/issues/114), [issue2](https://github.com/Puara/puara-module/issues/115), [issue3](https://github.com/Puara/puara-module/issues/116).

## Building From Source

### Dependencies

* [Git](https://git-scm.com/)
* [CMake](https://cmake.org/)
* C++ build tools for your system
  * **Windows**: [Visual Studio](https://visualstudio.microsoft.com/downloads/) or [mingw](https://www.mingw-w64.org/)
  * **Mac**: [Xcode](https://developer.apple.com/xcode/)
  * **Linux**: [gcc](https://gcc.gnu.org/) or [clang](https://clang.llvm.org/) (one or both are usually pre-installed on most distros)

### Instructions

1. Clone this repo and navigate to it in your terminal (e.g., `git clone https://github.com/IDMIL/idmil-puara-serial-library.git && cd idmil-puara-serial-library`)
2. Recursively pull submodules by running `git submodule update --init --recursive` (it may take a while for the `json` library to pull!)
3. Configure: `cmake -B build`
4. Build: `cmake --build build`

The build products include the library itself (on Mac, it's `build/libSerialManager.a`) and a command-line tool executable (`build/SerialTool`).

## Examples

First, you need to scan for connected Puara devices. This gives you a list of ports that you can send commands to.

```C++
#include <SerialManager.h>

std::vector<PuaraAPI::SerialManager::SerialPort> ports; // initialize as an empty vector
PuaraAPI::SerialManager::scan(ports); 
// after a brief 1 second maximum block, ports will contain all serial ports connected to Puara devices
```

`PuaraAPI::SerialManager::SerialPort` is a wrapper for `std::shared_ptr<serial::Serial>`. You can consider this a black box for abstraction purposes, but if you'd like to learn more about how this works, see the [serial library documentation](http://wjwwood.io/serial/doc/1.1.0/classserial_1_1_serial.html).

Once you have a port, you can send a Puara command to it using `sendCommand`:

```C++
if (!ports.empty()) {
  std::string name = PuaraAPI::SerialManager::sendCommand(ports[0], "whatareyou");
}
```

If you have multiple ports you want to send a single command to, `sendCommandMultiple` will do this for you in parallel. The main thread will block until all threads are finished (up to 1 second).

```C++
if (ports.size() >= 2) {
  std::vector<std::string> names = PuaraAPI::SerialManager::sendCommandMultiple(ports, "whatareyou");
} 
```

This library also contains one complex Puara operation named `changeConfig`. This changes the configuration of a Puara 0.3 device by sending multiple commands in series and managing error states automatically. You need to supply a `nlohmann::json` ([see documentation here](https://json.nlohmann.me/)) to configure it properly.

```C++
json config;

config["wifiSSID"] = "my_silly_ssid";
config["wifiPSK"] = "my_silly_password";

PuaraAPI::SerialManager::changeConfig(ports[0], config);
// this can block for a long time, so it's probably best to run it in parallel and monitor from a different thread.
```

## Libraries & Credits

* [serial](https://github.com/wjwwood/serial): Sending & receiving serial messages in native C++
* [json](https://github.com/nlohmann/json): Neatly handling JSON config data in C++
* [Puara firmware](https://github.com/Puara/puara-module): Targeted devices
* [puara-serial-manager](https://github.com/Puara/puara-serial-manager): Python predacessor to this tool

Special thanks to Edu Meneses, Marcelo Wanderley, Paul Buser, and Linnea Kirby for their help on this project. 

This software was developed by Ian Doherty at the [Input Devices and Music Interaction Laboratory (IDMIL)](https://www.idmil.org/) for use in the [T-Tree v3](https://github.com/foobiane/T-Tree-v3) in the fall of 2026. Part of a joint collaboration between the IDMIL and the [Société des Arts Technologiques (SAT)](https://sat.qc.ca/) for the [Puara](https://github.com/puara) project.
