# lcdspi jako STATIC library
# Kluczowe: pico_stdlib, hardware_spi, hardware_gpio jako PRIVATE
# żeby nie powodować duplikacji symboli gdy target linkuje je osobno
add_library(lcdspi STATIC
    lcdspi.c
)

target_include_directories(lcdspi PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)

# PRIVATE = zależności nie propagują się do targetów które linkują lcdspi
# Dzięki temu diag i ui mogą linkować pico_stdlib osobno bez kolizji
target_link_libraries(lcdspi PRIVATE
    pico_stdlib
    hardware_spi
    hardware_gpio
)
