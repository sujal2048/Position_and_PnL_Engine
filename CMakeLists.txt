cmake_minimum_required(VERSION 3.14)
project(PnLEngine VERSION 1.0 LANGUAGES CXX)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 NEW)
endif()
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Boost (includes Asio, Beast, JSON)
find_package(Boost 1.70 REQUIRED COMPONENTS system)

# Google Test (download if not found)
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# Executable
add_executable(pnl_engine
    src/main.cpp
    src/position_engine.cpp
    src/websocket_server.cpp
)
target_link_libraries(pnl_engine PRIVATE Boost::system)

# Unit tests
add_executable(tests
    tests/test_position_engine.cpp
    src/position_engine.cpp
)
target_link_libraries(tests PRIVATE Boost::system GTest::gtest_main)
