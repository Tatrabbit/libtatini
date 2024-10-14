#
# Project configuration
#
#cmake_minimum_required(VERSION 2.8.12...3.29 )
cmake_minimum_required(VERSION 3.20...3.29)
set(CMAKE_C_STANDARD 11)

# Project Name
project(bini-knife)

# Version Number
execute_process(
        COMMAND git rev-parse HEAD
        OUTPUT_VARIABLE GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(APP_VERSION_NUMBER ${GIT_REVISION})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(src/libbini)