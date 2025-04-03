cmake_minimum_required(VERSION 3.10)
project(CrowServer)

set(CMAKE_CXX_STANDARD 17)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/Crow/include)

# Find Boost
find_package(Boost REQUIRED)

# Find PostgreSQL (libpq) and libpqxx
find_package(PostgreSQL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(PQXX REQUIRED libpqxx)

include_directories(${Boost_INCLUDE_DIRS} ${PostgreSQL_INCLUDE_DIRS} ${PQXX_INCLUDE_DIRS})
link_directories(${PostgreSQL_LIBRARY_DIRS} ${PQXX_LIBRARY_DIRS})

# Add executable
add_executable(server main.cpp db_connection.cpp)
target_link_libraries(server PRIVATE Boost::boost ${PostgreSQL_LIBRARIES} ${PQXX_LIBRARIES})
