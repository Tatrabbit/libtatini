configure_file(defines.in.h
        ${CMAKE_BINARY_DIR}/src/defines.h
        FILE_PERMISSIONS OWNER_READ)

add_executable(tatini
        main.c
        cli_args.c
)

target_include_directories(tatini
        PRIVATE ${CMAKE_BINARY_DIR}/src
)

target_link_libraries(tatini PRIVATE libtatini)