# puara-serial-library: Native C++ library for managing Puara serial devices in Arduino-agnostic software

## Description

idmil-puara-serial-library is a basic C++ serial library designed to allow for abstracted serial communication between a host machine running this software and a device running Puara firmware.

Parts of this library implicitly target Puara version 0.3 (particularly `SerialManager::changeConfig` in `src/SerialManager.cpp`) as this is the version used by the [T-Stick](https://github.com/IDMIL/T-Stick), the DMI this library was developed for. Future versions of Puara will address certain serial bugs, which will necessitate an update of this both this library *and* the T-Stick firmware. See the targeted changes here: [issue1](https://github.com/Puara/puara-module/issues/114), [issue2](https://github.com/Puara/puara-module/issues/115), [issue3](https://github.com/Puara/puara-module/issues/116).

## Installation

[Coming soon]

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
2. Run `cmake -B build`
3. Run `cmake --build build`

The build products include the library itself (on Mac, it's `build/libSerialManager.dylib`) and a command-line tool executable (`build/SerialTool`).

## Libraries & Credits

* [serial](https://github.com/wjwwood/serial): Sending & receiving serial messages in native C++
* [json](https://github.com/nlohmann/json): Neatly handling JSON config data in C++
* [Puara firmware](https://github.com/Puara/puara-module): Targeted devices
* [puara-serial-manager](https://github.com/Puara/puara-serial-manager): Python predacessor to this tool

Special thanks to Edu Meneses, Marcelo Wanderley, Paul Buser, and Linnea Kirby for their help on this project. 

This software was developed by Ian Doherty at the [Input Devices and Music Interaction Laboratory (IDMIL)](https://www.idmil.org/) for use in the [T-Tree v3](https://github.com/foobiane/T-Tree-v3) in the fall of 2026. Part of a joint collaboration between the IDMIL and the [Société des Arts Technologiques (SAT)](https://sat.qc.ca/) for the [Puara](https://github.com/puara) project.