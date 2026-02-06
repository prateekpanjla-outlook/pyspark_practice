cmake_minimum_required(VERSION 3.15)
project(sql-practice-load-tester)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(CURL REQUIRED)

# Include directories
include_directories(${CURL_INCLUDE_DIRS})

# Source files
set(SOURCES
    src/main.cpp
    src/load_tester.cpp
)

# Executable
add_executable(load-tester ${SOURCES})

# Link libraries
target_link_libraries(load-tester ${CURL_LIBRARIES} pthread)

# Compiler flags
target_compile_options(load-tester PRIVATE
    -Wall -Wextra -O3
)
