cmake_minimum_required(VERSION 3.10)
project(traffic_simulator)

set(CMAKE_CXX_STANDARD 17)

find_package(PostgreSQL REQUIRED)
find_package(Threads REQUIRED)

add_executable(traffic_simulator
    main.cpp
    db.cpp
    simulator.cpp
)

target_include_directories(traffic_simulator PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_link_libraries(traffic_simulator PRIVATE ${PostgreSQL_LIBRARIES} Threads::Threads)
