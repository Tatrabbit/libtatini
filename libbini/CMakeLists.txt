cmake_minimum_required(VERSION 3.1)

add_library(libbini STATIC
        bini.c
        bini_infos.c
        mempool.c
)

target_include_directories(libbini INTERFACE include)