add_library(libbini STATIC
        bini_infos.c
        memchunk.c
        tat_bini.c
        bini_types.h
)

target_include_directories(libbini PUBLIC ${CMAKE_SOURCE_DIR}/include)