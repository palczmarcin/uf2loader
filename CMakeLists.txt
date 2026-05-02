cmake_minimum_required(VERSION 3.13)

# Wersja
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

# Pico SDK
include(pico_sdk_import.cmake)

project(uf2loader C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# --- Pimoroni Pico SDK ---
set(PIMORONI_PICO_PATH ${CMAKE_CURRENT_LIST_DIR}/pimoroni-pico)

# Dodaj źródła ST7789 bezpośrednio — bez cmake Pimoroni
add_library(st7789 STATIC
    ${PIMORONI_PICO_PATH}/drivers/st7789/st7789.cpp
)
target_include_directories(st7789 PUBLIC
    ${PIMORONI_PICO_PATH}
    ${PIMORONI_PICO_PATH}/drivers/st7789
    ${PIMORONI_PICO_PATH}/common/pimoroni_common
    ${PIMORONI_PICO_PATH}/common/pimoroni_bus
)
target_link_libraries(st7789
    pico_stdlib
    hardware_spi
    hardware_pwm
    hardware_dma
)

add_library(pico_graphics STATIC
    ${PIMORONI_PICO_PATH}/libraries/pico_graphics/pico_graphics.cpp
    ${PIMORONI_PICO_PATH}/libraries/pico_graphics/pico_graphics_pen_1bit.cpp
    ${PIMORONI_PICO_PATH}/libraries/pico_graphics/pico_graphics_pen_p2.cpp
    ${PIMORONI_PICO_PATH}/libraries/pico_graphics/pico_graphics_pen_p4.cpp
    ${PIMORONI_PICO_PATH}/libraries/pico_graphics/pico_graphics_pen_p8.cpp
    ${PIMORONI_PICO_PATH}/libraries/pico_graphics/pico_graphics_pen_rgb332.cpp
    ${PIMORONI_PICO_PATH}/libraries/pico_graphics/pico_graphics_pen_rgb565.cpp
    ${PIMORONI_PICO_PATH}/libraries/pico_graphics/pico_graphics_pen_rgb888.cpp
)
target_include_directories(pico_graphics PUBLIC
    ${PIMORONI_PICO_PATH}
    ${PIMORONI_PICO_PATH}/libraries/pico_graphics
    ${PIMORONI_PICO_PATH}/drivers/st7789
    ${PIMORONI_PICO_PATH}/common/pimoroni_common
    ${PIMORONI_PICO_PATH}/common/pimoroni_bus
)
target_link_libraries(pico_graphics
    pico_stdlib
    st7789
)

# --- Podkatalogi uf2loader ---
add_subdirectory(common/bootloader)
add_subdirectory(common/lcdspi)
add_subdirectory(common/petit_fatfs)
add_subdirectory(stage3)
add_subdirectory(ui)
add_subdirectory(diag)
