cmake_minimum_required(VERSION 3.16)
project(sql_watcher VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile_commands.json for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PostgreSQL_ROOT "/opt/homebrew/opt/postgresql@17")
find_package(PostgreSQL REQUIRED)
message(STATUS "PostgreSQL include: ${PostgreSQL_INCLUDE_DIRS}")
message(STATUS "PostgreSQL libs: ${PostgreSQL_LIBRARIES}")

include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.14
)
FetchContent_MakeAvailable(pugixml)

set(SOURCES
    src/main.cpp
    src/config.cpp
    src/connection.cpp
    src/listener.cpp
    src/dispatcher.cpp
    src/rules/rule_loader.cpp
    src/handlers/sensor_handler.cpp
    src/handlers/signal_handler.cpp
)

add_executable(watcher ${SOURCES})

target_include_directories(watcher PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PostgreSQL_INCLUDE_DIRS}
)

target_link_libraries(watcher
    ${PostgreSQL_LIBRARIES}
    nlohmann_json::nlohmann_json
    pugixml
    pthread
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(watcher PRIVATE -Wall -Wextra -Wpedantic)
endif()
