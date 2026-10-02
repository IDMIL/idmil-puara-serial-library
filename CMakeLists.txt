cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 20) # needed for std::format, std::filesystem
set(CMAKE_CXX_FLAGS "-Wall")

project(idmil-puara-serial-library)

# JSON library
add_subdirectory(libs/json)

# Serial library
add_library(serial STATIC)

if (APPLE)
    set(SerialImpl libs/serial/src/impl/unix.cc)
    set(SerialListPortsImpl libs/serial/src/impl/list_ports/list_ports_osx.cc)
elseif (UNIX)
    set(SerialImpl libs/serial/src/impl/unix.cc)
    set(SerialListPortsImpl libs/serial/src/impl/list_ports/list_ports_linux.cc)
else ()
    set(SerialImpl libs/serial/src/impl/win.cc)
    set(SerialListPortsImpl libs/serial/src/impl/list_ports/list_ports_win.cc)
endif ()

target_sources(serial
    PRIVATE
        libs/serial/src/serial.cc
        libs/serial/include/serial/serial.h
        libs/serial/include/serial/v8stdint.h
        ${SerialImpl}
        ${SerialListPortsImpl}
)

if (APPLE)
    target_link_libraries(serial
        "-framework IOKit"
        "-framework Foundation"
    )
elseif (UNIX)
    target_link_libraries(serial
        rt
        pthread
    )
else ()
    target_link_libraries(serial
        setupapi
    )
endif ()

target_include_directories(serial 
    PUBLIC
        libs/serial/include
)

# SerialManager library
add_library(SerialManager SHARED)
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