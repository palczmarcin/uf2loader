add_library(lcdspi STATIC
    lcdspi.c
)

target_include_directories(lcdspi PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(lcdspi
    pico_stdlib
    hardware_spi
    hardware_gpio
)
