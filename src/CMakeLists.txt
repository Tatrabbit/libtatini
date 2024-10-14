configure_file(defines.in.h ${CMAKE_BINARY_DIR}/src/defines.h FILE_PERMISSIONS OWNER_READ)

add_executable(bini-knife
        main.c defines.h
)

target_include_directories(bini-knife PRIVATE ${CMAKE_BINARY_DIR}/src)