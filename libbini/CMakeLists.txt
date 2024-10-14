add_library(libbini STATIC
        tat_bini.c memchunk.c
)

target_include_directories(libbini PUBLIC ../include)
target_sources(libbini PUBLIC ../include/tat_bini.h)