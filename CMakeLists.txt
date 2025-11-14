cmake_minimum_required(VERSION 3.10)
project(fileverse)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include your header directories
include_directories(include/core)
include_directories(source)

# Automatically find all .cpp files inside source/
file(GLOB_RECURSE FILEVERSE_SOURCES
    "source/*.cpp"
)

# Print found .cpp files (debug)
message(STATUS "Found source files:")
foreach(f ${FILEVERSE_SOURCES})
    message(STATUS " - ${f}")
endforeach()

# Build server
add_executable(fileverse_server
    ${FILEVERSE_SOURCES}
)

target_link_libraries(fileverse_server
    pthread
)
