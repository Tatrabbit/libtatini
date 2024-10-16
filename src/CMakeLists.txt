configure_file(defines.in.h ${CMAKE_BINARY_DIR}/src/defines.h FILE_PERMISSIONS OWNER_READ)

add_executable(bini-knife
        defines.h
        main.c
        ../include/tat_bini_infos.h
)
target_include_directories(bini-knife PRIVATE ${CMAKE_BINARY_DIR}/src)
target_link_libraries(bini-knife libbini)