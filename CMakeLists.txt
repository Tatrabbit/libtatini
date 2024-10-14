#
# Project configuration
#
cmake_minimum_required(VERSION 2.8.12...3.29)
set(CMAKE_C_STANDARD 23)

# Project Name
project(bini-knife)


#
# Version Number
#
execute_process(
        COMMAND git rev-parse HEAD
        OUTPUT_VARIABLE GIT_REVISION
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(APP_VERSION_NUMBER ${GIT_REVISION})


#
# GCC On/Off
#
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    set(CMAKE_GCC_LIKE ON)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_GCC_LIKE ON)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(CMAKE_GCC_LIKE OFF)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_GCC_LIKE OFF)
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(src)
add_subdirectory(src/libbini)