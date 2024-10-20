cmake_minimum_required(VERSION 3.1)

add_library(libbini STATIC
        tat_bini.c
        bini_infos.c
)

target_link_libraries(libbini PUBLIC libtat_c)
target_include_directories(libbini PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)