cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20) # needed for std::format, std::filesystem
set(CMAKE_CXX_FLAGS "-Wall")

project(puara-serial-library)

# GetComPortList library
add_library(GetComPortList)
target_sources(GetComPortList
    PRIVATE
        libs/GetComPortList/GetComPortList.cpp
)
target_include_directories(GetComPortList
    PUBLIC
        libs/GetComPortList
)

# JSON library
add_subdirectory(libs/json)

# Serial library
add_library(serial)

IF (WIN32)
    set(SerialImpl libs/serial/src/impl/win.cc)
ELSE()
    set(SerialImpl libs/serial/src/impl/unix.cc)
ENDIF()

target_sources(serial
    PRIVATE
        libs/serial/src/serial.cc
        ${SerialImpl}
)

target_include_directories(serial 
    PUBLIC
        libs/serial/include
)

# SerialManager library
add_library(SerialManager)
target_sources(SerialManager
    PRIVATE
        src/SerialManager.cpp
    PUBLIC
        FILE_SET serialManagerHeader
        TYPE HEADERS
        BASE_DIRS
            src
        FILES
            src/SerialManager.h
)
target_link_libraries(SerialManager
    PUBLIC
        serial
        nlohmann_json::nlohmann_json
        GetComPortList
)

# SerialTool executable
add_executable(SerialTool)
target_sources(SerialTool
    PRIVATE
        src/SerialTool.cpp
)
target_link_libraries(SerialTool
    PRIVATE
        SerialManager
)