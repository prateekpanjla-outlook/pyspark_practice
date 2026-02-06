cmake_minimum_required(VERSION 3.15)
project(sql-practice-server VERSION 1.0.0 LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use old C++11 ABI to match DuckDB library
add_compile_options(-D_GLIBCXX_USE_CXX11_ABI=0)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Platform-specific configuration
if(WIN32)
    # Windows configuration
    set(DUCKDB_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/duckdb.lib)
    set(DUCKDB_DLL ${CMAKE_CURRENT_SOURCE_DIR}/duckdb.dll)
    set(DUCKDB_EXTRA_LIB "")
elseif(UNIX)
    # Linux configuration - use project-local DuckDB library
    set(DUCKDB_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/libduckdb.so")
    set(DUCKDB_DLL "")
    # Link libraries properly using CMake's threading support
    set(DUCKDB_EXTRA_LIB "dl")
    # Set the library path
    set(DUCKDB_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
    # Find pthread package
    find_package(Threads REQUIRED)
endif()

message(STATUS "Platform: ${CMAKE_SYSTEM_NAME}")
message(STATUS "DuckDB Library: ${DUCKDB_LIBRARY}")

# Find packages
find_package(Threads REQUIRED)

# Oat++ - Use local dependency if available, otherwise download
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/oatpp/CMakeLists.txt")
    set(OATPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps/oatpp)
    message(STATUS "Using local Oat++ from deps/")
else()
    include(FetchContent)
    set(OATPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        oatpp
        GIT_REPOSITORY https://github.com/oatpp/oatpp.git
        GIT_TAG 1.3.0
    )
    FetchContent_MakeAvailable(oatpp)
    message(STATUS "Downloaded Oat++ from GitHub")
endif()

# nlohmann/json - Use local dependency if available, otherwise download
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/deps/json/single_include/nlohmann/json.hpp")
    # Header-only - just add include path
    message(STATUS "Using local nlohmann/json from deps/")
else()
    include(FetchContent)
    FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )
    FetchContent_MakeAvailable(json)
    message(STATUS "Downloaded nlohmann/json from GitHub")
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/core/session_manager.cpp
    src/core/config.cpp
    src/core/duckdb_instance_manager.cpp
    src/db/duckdb_executor.cpp
    src/db/question_loader.cpp
    src/db/embedded_questions.cpp
    src/http/http_server.cpp
)

# Header files
set(HEADERS
    src/include/session_manager.hpp
    src/include/sql_executor.hpp
    src/include/question_loader.hpp
    src/include/http_server.hpp
    src/include/duckdb_instance_manager.hpp
    src/include/config.hpp
)

# Create executable
add_executable(sql-practice-server ${SOURCES} ${HEADERS})

# Link libraries - link DuckDB first before other libraries
# nlohmann/json is header-only, so we only include it, not link it
target_link_libraries(sql-practice-server
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/libduckdb.so"
        oatpp
        ${DUCKDB_EXTRA_LIB}
        Threads::Threads
        rt
)

# Include directories
target_include_directories(sql-practice-server
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/oatpp/src
        ${CMAKE_CURRENT_SOURCE_DIR}/deps/json/single_include
)

# Platform-specific post-build
if(WIN32)
    # Copy DuckDB DLL to output directory
    add_custom_command(TARGET sql-practice-server POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${DUCKDB_DLL}
            $<TARGET_FILE_DIR:sql-practice-server>
        COMMENT "Copying DuckDB DLL to output directory"
    )
endif()

# Installation
install(TARGETS sql-practice-server
    RUNTIME DESTINATION bin
)

# Tests (optional)
option(BUILD_TESTS "Build unit tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Docker build (only if Docker is available)
if(UNIX)
    find_program(DOCKER_COMMAND docker)
    if(DOCKER_COMMAND)
        add_custom_target(docker-build
            COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target sql-practice-server
            COMMAND ${DOCKER_COMMAND} build -t sql-practice-cpp -f ${CMAKE_CURRENT_SOURCE_DIR}/docker/Dockerfile ${CMAKE_CURRENT_SOURCE_DIR}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Building Docker image"
        )
    endif()
endif()
