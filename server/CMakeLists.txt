cmake_minimum_required(VERSION 3.10)
project(CrowServer)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/Crow/include)
include_directories(/usr/include)
include_directories(/opt/homebrew/include)

# Find and Link Boost
find_package(Boost REQUIRED)

# Manually set libpqxx paths
set(PQXX_INCLUDE_DIR "/opt/homebrew/include")
set(PQXX_LIB "/opt/homebrew/lib/libpqxx.dylib")
set(PQ_LIB "/opt/homebrew/lib/libpq.dylib")

include_directories(${PQXX_INCLUDE_DIR})
link_directories("/opt/homebrew/lib")

# Add executable
add_executable(server main.cpp db_connection.cpp)
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS} ${PQXX_INCLUDE_DIR})
target_link_libraries(server PRIVATE Boost::boost ${PQXX_LIB} ${PQ_LIB})
