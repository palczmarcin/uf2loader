add_library(lcdspi STATIC
    lcdspi.cpp
)

target_include_directories(lcdspi PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../../
)

target_link_libraries(lcdspi
    pico_stdlib
    hardware_spi
    hardware_gpio
    hardware_pwm
    hardware_dma
    st7789        # sterownik Pimoroni
    pico_graphics # biblioteka graficzna Pimoroni
)
