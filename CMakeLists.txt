cmake_minimum_required(VERSION 3.5)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_STANDARD 20) # needed for std::format, std::filesystem
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

project(idmil-puara-serial-library)

# JSON library
add_subdirectory(libs/json)

# Serial library
add_library(WJWWOOD_SERIAL STATIC)

if (APPLE)
    set(WJWWOOD_SERIAL_IMPL libs/serial/src/impl/unix.cc)
    set(WJWWOOD_SERIAL_LIST_PORTS_IMPL libs/serial/src/impl/list_ports/list_ports_osx.cc)
elseif (UNIX)
    set(WJWWOOD_SERIAL_IMPL libs/serial/src/impl/unix.cc)
    set(WJWWOOD_SERIAL_LIST_PORTS_IMPL libs/serial/src/impl/list_ports/list_ports_linux.cc)
else ()
    set(WJWWOOD_SERIAL_IMPL libs/serial/src/impl/win.cc)
    set(WJWWOOD_SERIAL_LIST_PORTS_IMPL libs/serial/src/impl/list_ports/list_ports_win.cc)
endif ()

target_sources(WJWWOOD_SERIAL
    PRIVATE
        libs/serial/src/serial.cc
        libs/serial/include/serial/serial.h
        libs/serial/include/serial/v8stdint.h
        ${WJWWOOD_SERIAL_IMPL}
        ${WJWWOOD_SERIAL_LIST_PORTS_IMPL}
)

if (APPLE)
    target_link_libraries(WJWWOOD_SERIAL
        "-framework IOKit"
        "-framework Foundation"
    )
elseif (UNIX)
    target_link_libraries(WJWWOOD_SERIAL
        rt
        pthread
    )
else ()
    target_link_libraries(WJWWOOD_SERIAL
        setupapi
    )
endif ()

target_include_directories(WJWWOOD_SERIAL 
    PUBLIC
        libs/serial/include
)

# Timer utility library
add_library(IDMIL_PUARA_SERIAL_LIBRARY_TIMER STATIC)
target_sources(IDMIL_PUARA_SERIAL_LIBRARY_TIMER
    PRIVATE
        util/Timer.cpp
)

target_include_directories(IDMIL_PUARA_SERIAL_LIBRARY_TIMER
    PUBLIC
        util
)

# SerialManager library
add_library(IDMIL_PUARA_SERIAL_LIBRARY_SERIAL_MANAGER STATIC)
target_sources(IDMIL_PUARA_SERIAL_LIBRARY_SERIAL_MANAGER
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
target_link_libraries(IDMIL_PUARA_SERIAL_LIBRARY_SERIAL_MANAGER
    PUBLIC
        WJWWOOD_SERIAL
        IDMIL_PUARA_SERIAL_LIBRARY_TIMER
        nlohmann_json::nlohmann_json
)

# SerialTool executable
add_executable(IDMIL_PUARA_SERIAL_LIBRARY_SERIAL_TOOL)
target_sources(IDMIL_PUARA_SERIAL_LIBRARY_SERIAL_TOOL
    PRIVATE
        src/SerialTool.cpp
)
target_link_libraries(IDMIL_PUARA_SERIAL_LIBRARY_SERIAL_TOOL
    PRIVATE
        IDMIL_PUARA_SERIAL_LIBRARY_SERIAL_MANAGER
)
