# Używamy OBJECT zamiast STATIC żeby uniknąć duplikacji symboli pico_stdlib
add_library(lcdspi OBJECT
    lcdspi.c
)

target_include_directories(lcdspi PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(lcdspi PUBLIC
    pico_stdlib
    hardware_spi
    hardware_gpio
)
