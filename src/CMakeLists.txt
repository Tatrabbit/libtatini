configure_file(defines.in.h
        ${CMAKE_BINARY_DIR}/src/defines.h
        FILE_PERMISSIONS OWNER_READ)

add_executable(bini-knife
        main.c
)

target_link_libraries(bini-knife PRIVATE libbini)