add_library(libtat_c STATIC
        memory_pool.c
        #        memory_pool.h
)

target_include_directories(libtat_c PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)