cmake_minimum_required(VERSION 3.16)
project(sql_watcher VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable compile_commands.json for LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------
# PostgreSQL Discovery
# -----------------------------
if(APPLE)
    find_program(BREW_EXECUTABLE brew)

    if(BREW_EXECUTABLE)
        execute_process(
            COMMAND ${BREW_EXECUTABLE} --prefix postgresql@17
            OUTPUT_VARIABLE BREW_PG_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        if(BREW_PG_PREFIX)
            set(PostgreSQL_ROOT "${BREW_PG_PREFIX}")
        endif()
    endif()
endif()

# Fallback common install paths
foreach(PG_PATH
    /opt/homebrew/opt/postgresql@17
    /usr/local/opt/postgresql@17
    /opt/homebrew
    /usr/local
)
    if(EXISTS "${PG_PATH}/include/postgresql/server")
        set(PostgreSQL_ROOT "${PG_PATH}")
        break()
    endif()
endforeach()

find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

message(STATUS "PostgreSQL include: ${PostgreSQL_INCLUDE_DIRS}")
message(STATUS "PostgreSQL libs: ${PostgreSQL_LIBRARIES}")

# -----------------------------
# Dependencies
# -----------------------------
include(FetchContent)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(json)

FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.14
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(pugixml)

# -----------------------------
# Sources
# -----------------------------
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

# -----------------------------
# Target
# -----------------------------
add_executable(watcher ${SOURCES})

target_include_directories(watcher PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(watcher PRIVATE
    PostgreSQL::PostgreSQL
    nlohmann_json::nlohmann_json
    pugixml
    Threads::Threads
)

# -----------------------------
# Warnings
# -----------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(watcher PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
