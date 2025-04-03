cmake_minimum_required(VERSION 3.10)
project(CrowServer)

set(CMAKE_CXX_STANDARD 17)

# Include Crow and ASIO
include_directories(${CMAKE_SOURCE_DIR}/Crow/include)
include_directories(/usr/include)  # For Linux/macOS
include_directories(/opt/homebrew/include)  # For macOS with Homebrew

# Add executable
add_executable(server main.cpp)

# Find and Link Boost
find_package(Boost REQUIRED)
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(server PRIVATE Boost::boost)
