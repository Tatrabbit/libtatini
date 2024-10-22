cmake_minimum_required(VERSION 3.1)

add_library(libtatini STATIC
        libtatini.c
        libtatini_infos.c
        libtat_mempool.c
)

target_include_directories(libtatini INTERFACE include)