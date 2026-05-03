cmake_minimum_required(VERSION 3.13)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)
set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

include(pico_sdk_import.cmake)

project(uf2loader C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

# --- Podkatalogi uf2loader ---
# Nasz własny lcdspi zawiera kompletny sterownik ST7789 SPI
# bez zależności od pimoroni-pico
add_subdirectory(common/bootloader)
add_subdirectory(common/lcdspi)
add_subdirectory(common/petit_fatfs)
add_subdirectory(stage3)
add_subdirectory(ui)
add_subdirectory(diag)
